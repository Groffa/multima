#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include "game_arts.h"

static int
GetFileSize(FILE *File)
{
    int Size = 0;
    fseek(File, 0, SEEK_END);
    Size = ftell(File);
    fseek(File, 0, SEEK_SET);
    return Size;
}

static artfile_entry_t
CreateEntry(const char *Filename)
{
    artfile_entry_t Entry = {};
    FILE *File = fopen(Filename, "rb");
    Entry.Size = GetFileSize(File);
    fclose(File);
    return Entry;
}

static void
WriteEntry(FILE *ArtFile, artfile_entry_t *Entry, u8 *Data)
{
    fwrite(Entry, sizeof(artfile_entry_t), 1, ArtFile);
    fwrite(Data, sizeof(char), Entry->Size, ArtFile);
}

static bool
IsAccepted(u8 C)
{
    return (C >= 'A' && C <= 'Z') ||
           (C >= 'a' && C <= 'z') ||
           (C >= '0' && C <= '9');
}

static void
ExtractEnumFriendlyFilename(char *Filename, char *BareFilename, uint BareFilenameSize = 0)
{
    int MaxLen = strlen(Filename);
    if (BareFilenameSize > 0 && MaxLen >= BareFilenameSize) {
        MaxLen = BareFilenameSize;
    }
    for (int i=0; i < MaxLen; ++i) {
        if (Filename[i] == '.') {
            break;
        }
        if (IsAccepted(Filename[i])) {
            BareFilename[i] = Filename[i];
        }
    }
}

static u8
Luminosity(u8 R, u8 G, u8 B)
{
    return (u8)(0.21f * R + 0.72f * G + 0.07f * B);
}

static artfile_entry_t
AppendImageTo(FILE *ArtFile, char *Filename)
{
    artfile_entry_t Entry = CreateEntry(Filename);
    
    FILE *File = fopen(Filename, "rb");
    BITMAPFILEHEADER BmpFileHeader = {0};
    fread(&BmpFileHeader, sizeof(BITMAPFILEHEADER), 1, File);
   
    BITMAPV5HEADER BmpV5Header = {0};
    fread(&BmpV5Header, sizeof(BITMAPV5HEADER), 1, File);
    
    assert(BmpV5Header.bV5BitCount == 32);

    //uint ImageSize = BmpV5Header.bV5Width * BmpV5Header.bV5Height;
    uint ImageSize = BmpV5Header.bV5SizeImage;
    u8 *ImageData = (u8 *)malloc(ImageSize);
    fseek(File, BmpFileHeader.bfOffBits, SEEK_SET);
    fread(ImageData, ImageSize, 1, File);
   
    uint NewImageSize = BmpV5Header.bV5Width * BmpV5Header.bV5Height;
    u8 *NewImageData = (u8 *)malloc(NewImageSize);
    uint DstIndex = 0;
    for (uint SrcIndex=0; SrcIndex < ImageSize;) {
        u8 A = *(ImageData + SrcIndex++);
        u8 B = *(ImageData + SrcIndex++);
        u8 G = *(ImageData + SrcIndex++);
        u8 R = *(ImageData + SrcIndex++);
        u8 ColorData = 0;
        if (R || G || B) {
            ColorData = Luminosity(R, G, B);
        }
        *(NewImageData + DstIndex++) = ColorData;
    }

    // Bottom-up bitmap? Then let's reverse it
    if (BmpV5Header.bV5Height > 0) {
        u8 *ReversedImageData = (u8 *)malloc(NewImageSize);
        u8 *Src = NewImageData;
        u8 *Dst = (u8 *)(ReversedImageData + NewImageSize - 1);
        for (uint i=0; i < NewImageSize; ++i) {
            *Dst-- = *Src++;
        }
        memcpy(NewImageData, ReversedImageData, NewImageSize);
        free(ReversedImageData);
    }

    // Entry.Size = BmpV5Header.bV5SizeImage;
    Entry.Type = ArtFile_bitmap;

    uint EntryNameIndex = 15;
    uint FilenameSize = strlen(Filename);
    bool FoundLastDot = false;
    for (uint i=FilenameSize - 1; i >= 0 && EntryNameIndex >= 0; --i) {
        char C = Filename[i];
        if (FoundLastDot) {
            if (IsAccepted(C)) {
                Entry.Name[EntryNameIndex--] = C;
            } else {
                break;
            }
        } else if (C == '.') {
            FoundLastDot = true;
        }
    }
    // Skip past those zeroes
    uint EntryNameStartsAt = 0;
    for (; Entry.Name[EntryNameStartsAt] == 0; ++EntryNameStartsAt);
    for (uint i=EntryNameStartsAt; i < 16; ++i) {
        Entry.Name[i-EntryNameStartsAt] = Entry.Name[i];
        Entry.Name[i] = 0;
    }

    Entry.Size = BmpV5Header.bV5Width * BmpV5Header.bV5Height;
    Entry.Dim.Width = BmpV5Header.bV5Width;
    Entry.Dim.Height = BmpV5Header.bV5Height;

    // Need to set before writing entry to ArtFile
    WriteEntry(ArtFile, &Entry, NewImageData);

    free(ImageData);
    free(NewImageData);
    fclose(File);
    
    return Entry;
}

static unsigned int
FindAndPack(FILE *ArtFile, FILE *ArtIncludeFile, char *Directory, char *Pattern)
{
    unsigned EntryCount = 0;
    char FullPattern[MAX_PATH] = {0};
    strncpy(FullPattern, Directory, strlen(Directory));
    strcat(FullPattern, Pattern);

    WIN32_FIND_DATA FindData = {0};
    HANDLE Handle = FindFirstFile(FullPattern, &FindData);
    if (Handle == INVALID_HANDLE_VALUE) {
        return -1;
    }
    do {
        char FullFilename[MAX_PATH] = {0};
        strncpy(FullFilename, Directory, strlen(Directory));
        strcat(FullFilename, FindData.cFileName);
        uint EntryOffset = ftell(ArtFile);
        AppendImageTo(ArtFile, FullFilename);
        
        // Update include file
        char BareFilename[MAX_PATH] = {0};
        ExtractEnumFriendlyFilename(FindData.cFileName, BareFilename);
        fprintf(ArtIncludeFile, "GameItem_%s = 0x%x,\n", BareFilename, EntryOffset);

        ++EntryCount;
    } while (FindNextFile(Handle, &FindData) != 0);
    FindClose(Handle);
    return EntryCount;
}

int
main(int argc, char **argv)
{
    int Result = 0;

    if (argc != 3) {
        printf("Usage: %s <arts dir> <dest dir>\n", argv[0]);
        return -1;
    }
    
    char ArtFilename[255] = {0};
    char ArtIncludeFilename[255] = {0};
    strncpy(ArtFilename, argv[2], strlen(argv[2]));
    strncpy(ArtIncludeFilename, ArtFilename, strlen(ArtFilename));
    strcat(ArtFilename, ARTFILE_DEFAULT_FILENAME ".art");
    strcat(ArtIncludeFilename, ARTFILE_DEFAULT_FILENAME ".h");

    // Create new art file. The header will be written AFTER we've processed
    // all files
    artfile_header_t Header = {};
    Header.Magic = ArtFileMagic_v1;
    FILE *ArtFile = fopen(ArtFilename, "wb");
    fseek(ArtFile, sizeof(artfile_header_t), SEEK_SET);

    // Generate include file
    FILE *ArtIncludeFile = fopen(ArtIncludeFilename, "w");
    fputs("#ifndef GAME_ITEMS_H\n\n", ArtIncludeFile);
    fputs("// These are offsets into the memory where the\n", ArtIncludeFile);
    fputs("// specific artfile_entry_t can be found.\n", ArtIncludeFile);
    fputs("enum game_item_e {\n", ArtIncludeFile);

    // Find and pack our groups
    int EntryCount = FindAndPack(ArtFile, ArtIncludeFile, argv[1], "*.bmp");
    DWORD LastError = GetLastError();
    if (EntryCount < 0 && LastError != 0) {
        printf("Error packing (err %i)\n", GetLastError());
        Result = -1;
    }
    Header.EntryCount += EntryCount;
    
    // Rewind tape and write header
    fseek(ArtFile, 0, SEEK_SET);
    fwrite(&Header, sizeof(artfile_header_t), 1, ArtFile);

    // Write footer of include file
    fputs("};\n\n", ArtIncludeFile);
    fputs("#define GAME_ITEMS_H\n#endif", ArtIncludeFile);

    // At the end of the day
    fclose(ArtFile);
    fclose(ArtIncludeFile);

    return Result;
}
