# Slicer #

Drag and drop .bvf or .rawbvf file in the slicer window to load the data.

Lowres slices are first displayed and are progressively replaced with higher resolution ones.  The level of subsampling is displayed.  Only the visible part of the slice is shown in higher resolution.


Press the following keys within the image window.
| **Key** | **Description** |
|:--------|:----------------|
| **Ctrl+0** | Display full resolution slice. |
| **Ctrl++** | Zoom in.        |
| **Ctrl+-** | Zoom out.       |

Use mouse wheel or up/down arrow keys in slider window to change current slice position.  For arrow keys to work the keyboard focus has to be in the slider window - mouse click within a window will change the focus to that window.

The colours are mapped in a linear fashion.  Move mouse to colour widget and press space bar to change colour scheme.  Left click will create new colour nodes and right click will remove them. Double click to change colour for that colour node.  Press **f** to flip (reverse) the colour range.

Change the focus to histogram window and press space bar to bring up a hialog to change min and max values for raw data sets.  Current min and max values will be shown in the panel.