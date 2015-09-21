Trying out new file format for volumetric data.  Data will be stored as blocks (with additional padding if required).

The importer application will import data and save it in blocked format.

The replicator application is similar to importer except the data is replicated 8 times in order to create a larger dataset for testing.

The slicer application reads the data stored in blocked format and extracts slices in any of X/Y/Z directions.  You might have to tweek the **m\_maxCacheSize** defined in **blockfilereader.cpp** in order to make maximum use of available memory.

  * [BlockedFileFormat](BlockedFileFormat.md)
  * [BlockFileWriter](BlockFileWriter.md)
  * [Slicer](Slicer.md)