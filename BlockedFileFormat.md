# Proposed blocked file structure #

The blocked data is currently stored in raw format.  Importer program reads each slice and passes it onto **blockfilewriter**, which manages the storage of the data on to disk.

The data is storage is dividec into multiple files. The main file had **.bvf** extention. This file is saved in xml format.  Following is an example of one such file.
```
<!DOCTYPE Drishti_Header>
<PvlDotNcFileHeader>
  <rawfile>sb.rawbvf</rawfile>
  <voxeltype>unsigned short</voxeltype>
  <gridsize>494 832 832</gridsize>
  <voxelunit>no units</voxelunit>
  <voxelsize>1 1 1</voxelsize>
  <description>Information about volume</description>
  <maxfilesize>1073741824</maxfilesize>
  <blocksize>64</blocksize>
  <border>0</border>
  <rawmap>0 2476.05 </rawmap>
  <pvlmap>0 255 </pvlmap>
</PvlDotNcFileHeader>
```

The lowres version of the data is stored in .bvf.lowres file.  Currently the volume is subsampled so that it fits in 128Mb.

The data is split into blocks - the block size is specified by the user (32/64/128). The data files that store the actual voxel data have .bvf._blocksize_.001, bvf._blocksize_.002, ... extention.  For e.g. test.bvf.32.001, test.bvf.32.002 and so on.

Each block is subsampled and stored in appropriate files (e.g. test.bvf.16.001, ...).  The level of thie block subsampling is decided by level of subsampling required to generate the lowres volume (stored in .bvf.lowres file).

If a block is found to have a constant value, only a single value is stored for the block instead of storing the entire block.  This information regarding constant values along with block offsets and blocks in a file is stored in (yet) another file with .bvf.dict extention.

When the raw file is saved in blocked format, the file extension is changed to **.rawbvf** instead of **.bvf**.

Please refer to [BlockFileWriter](BlockFileWriter.md) for details.