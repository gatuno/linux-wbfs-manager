# Usage #

## Device Access ##

To use this program you must have write access to your raw block devices.
On most systems, this means having to run the program as root (or manually
changing device permissions). You may use "`sudo`"; for example, in Ubuntu
you might run the program as:

> `sudo ./wbfs_gtk`

## Basic Usage ##

When you run the program, it should show the contents of the first WBFS partition it finds (if any).

The left pane shows information about the loaded WBFS partition (the
list of discs and the space usage). The right pane shows your
filesystem (so you can select ISO files to add).

The available operations are:

  * **Add an ISO file to WBFS partition**: in the right panel, navigate to the directory that contains the ISO and select file and click "Add ISO" (or simply double-click the file).
  * **Extract a disc from the WBFS to an ISO file**: select the disc in the right panel and click "Extract ISO". The ISO file will be written to the directory currently viewed in the right panel.
  * **Remove a disc from the WBFS**: select the disc in the left panel and click "Remove Disc".
  * **Format partition as WBFS**: select the device and click the menu "Tools -> Initialize WBFS partition".

## Selecting Other Partitions ##

You can change the currently displayed WBFS partition by selecting a device in the upper left corner of the window (the dropdown list named "Device") and clicking the green arrow on its right.

(If it shows an error about "bad magic", most likely the device you
selected doesn't contain a WBFS partition.)

The program lists devices `/dev/hd*` and `/dev/sd*`, excluding the devices
it thinks are mounted (according to /proc/mounts). You may choose do
list the mounted devices anyway by selecting this option in the menu "View ->
Show mounted devices".