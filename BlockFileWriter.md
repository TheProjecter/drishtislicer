# BlockFileWriter #

BlockFileWriter class manages the creation and writing of blocked files to disk.

basefilename, grid sizes, voxeltype, blocksize information is passed onto the writer.  The writer collects a block of **blocksize** slices before chopping it up into **blocksize** blocks for storing on disk.  Each block may be subsampled and stored.

| **setSlice** | Collects the slices and creates lowres volume.|
|:-------------|:----------------------------------------------|
| **genSliceBlocks** | Chops up the slice block into _blocksize_ blocks.|
| **dumpSliceBlocks** | Saves blocks to disk.  In doing so it creates information about uniform blocks, block offsets and blocks within a file.|
| **saveDict** | Saves the dictionary - uniform info, block offsets and file blocks - along with the lowres volume.|

| **m\_ssvol** | Stores lowres volume. |
|:-------------|:----------------------|
| **m\_sliceAcc** | Accumulates slices.   |
| **m\_slice** | Stores fullres blocks and subsequent subsampled blocks. |
| **m\_uniform** | Stores constant block information. |
| **m\_blockOffset** | Stores block offsets within a file. |
| **m\_fileBlocks** | Stores number of blocks within a file. |

In the importer, **blockfilewriter** gets its input from **raw2pvl.cpp** in the routine **savePvl**.  **savePvl** uses **pvlFileManager** to save processed volume and **rawFileManager** to store raw data.

Routine **savePvlHeader** within **raw2pvl.cpp** create .bvf and .bvfraw files.