//  Term Project — Step 1 — VDI File Access
//  Goals
//  • Provide the five basic file I/O functions to access disk space inside a VDI file.
//  • Create a structure or class to contain the data necessary to implement the five functions.
//  Details
//  You’ll want a single entity — a structure or a class, either works — to represent a VDI file within your project. The intent is to collect the various data your project is going to need into one place for ease of use; if you’re using C++ (you are using C or C++, right?) then a class can also contain the basic I/O functions as methods.
//  What do you need to keep track of in a VDI file? To implement the five functions you’ll probably want at least these four items to be contained in your structure:
//  • The file descriptor for the VDI file. This is an int.
//  • A VDI header structure. Discussion of the structure can be found at
//  https://forums.virtualbox.org/viewtopic.php?t=8046; C structures for the header can be found in the VirtualBox source code.
//  • The VDI translation map. This is optional; include it if you are going to enable dynamic VDI files (they’re easy!), ignore it otherwise. This is just an array of integers, although you don’t know the size in advance. The size is given in the VDI header, so you’ll need to allocate this dynamically.
//  • A cursor. This is just an integer (of type size_t) that holds the location of the next byte to be read or written.
//  Once the structure / class is created, you’ll need to implement a VDI version of each of the five basic I/O functions.
//  • struct VDIFile *vdiOpen(char *fn)
//  Open the file whose name is given. The filename can be either a relative or absolute path.
//  The function returns a pointer to a dynamically created VDI file structure (see above), or a null pointer if there was an error. The function should load the header and the translation map, set the cursor to 0 and set the file descriptor to whatever was returned from the open() system
//  call.
//  If you’re using a class, then this can return a boolean to indicate success or failure of the open.
//  • void vdiClose(struct VDIFile *f)
//  Close the file whose pointer is given. Deallocate any dynamically created memory regions.
//  • ssize_t vdiRead(struct VDIFile *f,void *buf,size_t count)
//  Reads the given number of bytes from the given VDI file’s disk space, placing the bytes in the given buffer. The location of the first byte read is given by the cursor, which is relative to the start of the VDI file’s data space. Advance the cursor by the number of bytes read.
//  • ssize_t vdiWrite(struct VDIFile *f,void *buf,size_t count)
//  Writes the given number of bytes to the given file, starting at the cursor. Bytes are written sequentially and the cursor is advanced to the end of the written block. Bytes to be written are located in the given buffer.
//  • off_t vdiSeek(VDIFile *f,off_t offset,int anchor)
//  Move the cursor of the given file to the given location, based on the offset and anchor values.
//  If the resulting location is negative or larger than the disk size, do not change the value of the cursor.
//  If you are using a class, then the VDIFile * parameter is omitted.
//  The functions present the raw disk image as if it were the only data in the VDI file; all aspects of the VDI structure are to be hidden from the user. So, when the user wants to read, they only read from the disk space, never from the raw VDI file. Writing is similar. The cursor can only specify locations within the disk image.
//  If your project handles dynamic VDI files, then disk image should be presented as if all pages were laid out in the proper order, regardless of where they are in the VDI file. Missing pages must also be handled as if they were there.
//  You should also write a function that takes a pointer to a VDIFile as a parameter and displays its header fields in an easy-to-read manner. See example 1 below for a sample; your exact format may vary.
//  Suggestions
//  • vdiSeek() should only set the cursor in the VDIFile structure; it should not call lseek().
//  • Reading and writing should be done one page at a time. While there are bytes left to be read or written, do the following tasks:
//  1. Determine where within the current page reading or writing should begin.
//  2. Determine how many bytes should be read or written in the current page.
//  3. Determine the physical location of the page. This may involve page translation and/or page allocation. Questions: What if the page is not allocated? What if the page is marked as “all zeroes”?
//  4. Use lseek() to go to the proper location within the physical page.
//  5. Use read() or write() to read or write only the bytes within the current page.
//  6. Advance the cursor by the number of bytes read or written; subtract the number of bytes read or written from the number of bytes remaining.
//  • If you are planning to write into the filesystem, consider using the mmap()andmunmap()functions to load the VDI file header and translation map. These act like a write-through cache; they read the areas from the file into memory set up by the OS, and writing to them automagically writes back to the file.
//  Example 1
//  This is the output from mytestVDI()function on thegood-fixed-1k.vdifile.
//  Dump of VDI header:
//       Image name: [<<< Oracle VM VirtualBox Disk Image >>>
//                         ]
//        Signature: 0xbeda107f
//          Version: 1.01
//      Header size: 0x00000190  400
//       Image type: 0x00000002
//            Flags: 0x00000000
//      Virtual CHS: 0-0-0
//      Sector size: 0x00000200  512
//      Logical CHS: 260-16-63
//      Sector size: 0x00000200  512
//       Map offset: 0x00100000  1048576
//     Frame offset: 0x00200000  2097152
//       Frame size: 0x00100000  1048576
//  Extra frame size: 0x00000000  0
//     Total frames: 0x00000080  128
//  Frames allocated: 0x00000080  128
//        Disk size: 0x0000000008000000  134217728
//             UUID: 6fd7a9b3-5226-8740-aea7-506076e113b0
//   Last snap UUID: 6221a1ea-f266-e946-aeeb-4959370a749b
//        Link UUID: 00000000-0000-0000-0000-000000000000
//      Parent UUID: 00000000-0000-0000-0000-000000000000
//    Image comment:
//  Offset: 0x54
//    00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    0...4...8...c...
//   +-----------------------------------------------+  +----------------+
//  00|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|00|                |
//  10|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|10|                |
//  20|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|20|                |
//  30|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|30|                |
//  40|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|40|                |
//  50|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|50|                |
//  60|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|60|                |
//  70|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|70|                |
//  80|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|80|                |
//  90|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|90|                |
//  a0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|a0|                |
//  b0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|b0|                |
//  c0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|c0|                |
//  d0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|d0|                |
//  e0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|e0|                |
//  f0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|f0|                |
//   +-----------------------------------------------+  +----------------+
//           Unused: 0
//  Header in binary:
//  Offset: 0x0
//    00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    0...4...8...c...
//   +-----------------------------------------------+  +----------------+
//  00|3c 3c 3c 20 4f 72 61 63 6c 65 20 56 4d 20 56 69|00|<<< Oracle VM Vi|
//  10|72 74 75 61 6c 42 6f 78 20 44 69 73 6b 20 49 6d|10|rtualBox Disk Im|
//  20|61 67 65 20 3e 3e 3e 0a 00 00 00 00 00 00 00 00|20|age >>>         |
//  30|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|30|                |
//  40|7f 10 da be 01 00 01 00 90 01 00 00 02 00 00 00|40|                |
//  50|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|50|                |
//  60|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|60|                |
//  70|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|70|                |
//  80|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|80|                |
//  90|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|90|                |
//  a0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|a0|                |
//  b0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|b0|                |
//  c0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|c0|                |
//  d0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|d0|                |
//  e0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|e0|                |
//  f0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|f0|                |
//   +-----------------------------------------------+  +----------------+
//  Offset: 0x100
//    00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    0...4...8...c...
//   +-----------------------------------------------+  +----------------+
//  00|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|00|                |
//  10|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|10|                |
//  20|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|20|                |
//  30|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|30|                |
//  40|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|40|                |
//  50|00 00 00 00 00 00 10 00 00 00 20 00 00 00 00 00|50|                |
//  60|00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 00|60|                |
//  70|00 00 00 08 00 00 00 00 00 00 10 00 00 00 00 00|70|                |
//  80|80 00 00 00 80 00 00 00 6f d7 a9 b3 52 26 87 40|80|        o   R& @|
//  90|                                               |90|                |
//  a0|                                               |a0|                |
//  b0|                                               |b0|                |
//  c0|                                               |c0|                |
//  d0|                                               |d0|                |
//  e0|                                               |e0|                |
//  f0|                                               |f0|                |
//   +-----------------------------------------------+  +----------------+
//  Translation map:
//  Offset: 0x0
//  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    0...4...8...c...
//  +-----------------------------------------------+  +----------------+
//  00|00 00 00 00 01 00 00 00 02 00 00 00 03 00 00 00|00|                
// |
//  10|04 00 00 00 05 00 00 00 06 00 00 00 07 00 00 00|10|                
// 20|08 00 00 00 09 00 00 00 0a 00 00 00 0b 00 00 00|20|                
// 30|0c 00 00 00 0d 00 00 00 0e 00 00 00 0f 00 00 00|30|                
// 40|10 00 00 00 11 00 00 00 12 00 00 00 13 00 00 00|40|                
// 50|14 00 00 00 15 00 00 00 16 00 00 00 17 00 00 00|50|                
// 60|18 00 00 00 19 00 00 00 1a 00 00 00 1b 00 00 00|60|                
// 70|1c 00 00 00 1d 00 00 00 1e 00 00 00 1f 00 00 00|70|                
// |
//  |
//  |
//  |
//  |
//  |
//  |
//  80|20 00 00 00 21 00 00 00 22 00 00 00 23 00 00 00|80|    !   "   #   |
//  90|24 00 00 00 25 00 00 00 26 00 00 00 27 00 00 00|90|$   %   &   ’   |
//  a0|28 00 00 00 29 00 00 00 2a 00 00 00 2b 00 00 00|a0|(   )   *   +   |
//  b0|2c 00 00 00 2d 00 00 00 2e 00 00 00 2f 00 00 00|b0|,   -   .   /   |
//  c0|30 00 00 00 31 00 00 00 32 00 00 00 33 00 00 00|c0|0   1   2   3   |
//  d0|34 00 00 00 35 00 00 00 36 00 00 00 37 00 00 00|d0|4   5   6   7   |
//  e0|38 00 00 00 39 00 00 00 3a 00 00 00 3b 00 00 00|e0|8   9   :   ;   |
//  f0|3c 00 00 00 3d 00 00 00 3e 00 00 00 3f 00 00 00|f0|<   =   >   ?   |
//  +-----------------------------------------------+  +----------------+
//  Offset: 0x100
//  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    0...4...8...c...
//  +-----------------------------------------------+  +----------------+
//  00|40 00 00 00 41 00 00 00 42 00 00 00 43 00 00 00|00|@   A   B   C   |
//  10|44 00 00 00 45 00 00 00 46 00 00 00 47 00 00 00|10|D   E   F   G   |
//  20|48 00 00 00 49 00 00 00 4a 00 00 00 4b 00 00 00|20|H   I   J   K   |
//  30|4c 00 00 00 4d 00 00 00 4e 00 00 00 4f 00 00 00|30|L   M   N   O   |
//  40|50 00 00 00 51 00 00 00 52 00 00 00 53 00 00 00|40|P   Q   R   S   |
//  50|54 00 00 00 55 00 00 00 56 00 00 00 57 00 00 00|50|T   U   V   W   |
//  60|58 00 00 00 59 00 00 00 5a 00 00 00 5b 00 00 00|60|X   Y   Z   [   |
//  70|5c 00 00 00 5d 00 00 00 5e 00 00 00 5f 00 00 00|70|\   ]   ^   _   |
//  80|60 00 00 00 61 00 00 00 62 00 00 00 63 00 00 00|80|‘   a   b   c   |
//  90|64 00 00 00 65 00 00 00 66 00 00 00 67 00 00 00|90|d   e   f   g   |
//  a0|68 00 00 00 69 00 00 00 6a 00 00 00 6b 00 00 00|a0|h   i   j   k   |
//  b0|6c 00 00 00 6d 00 00 00 6e 00 00 00 6f 00 00 00|b0|l   m   n   o   |
//  c0|70 00 00 00 71 00 00 00 72 00 00 00 73 00 00 00|c0|p   q   r   s   |
//  d0|74 00 00 00 75 00 00 00 76 00 00 00 77 00 00 00|d0|t   u   v   w   |
//  e0|78 00 00 00 79 00 00 00 7a 00 00 00 7b 00 00 00|e0|x   y   z   {   |
//  f0|7c 00 00 00 7d 00 00 00 7e 00 00 00 7f 00 00 00|f0||   }   ~       
// |
//  +-----------------------------------------------+  +----------------+
//  Partition table from Master Boot Record:
//  Offset: 0x100
//    00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f    0...4...8...c...
//   +-----------------------------------------------+  +----------------+
//  00|                                               |00|                |
//  10|                                               |10|                |
//  20|                                               |20|                |
//  30|                                               |30|                |
//  40|                                               |40|                |
//  50|                                               |50|                |
//  60|                                               |60|                |
//  70|                                               |70|                |
//  80|                                               |80|                |
//  90|                                               |90|                |
//  a0|                                               |a0|                |
//  b0|                                          00 04|b0|                |
//  c0|01 04 83 01 82 02 00 08 00 00 00 f8 03 00 00 00|c0|                |
//  d0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|d0|                |
//  e0|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|e0|                |
//  f0|00 00 00 00 00 00 00 00 00 00 00 00 00 00      |f0|                |
//   +-----------------------------------------------+  +----------------+
//  Other Examples
//  The output for the other five sample VDI files is available in the repository in the filestep1.log
