# Installer Improvement

The bootstrap asks for the installation folder before creating logs or extracting
files. Cancellation returns without extraction. The selected folder is the
installation root; updating requires selecting that same folder. No global path
preference is stored beside the downloaded installer or in a fixed user folder.
The installed shortcut still opens the launcher directly.

Automated checks: real payload extracted into a selected Unicode/space path;
repeated extraction preserves a synthetic save; runtime hashes match; no game
folder appears beside the test executable. No game was launched. The native
folder-picker interaction itself has not been manually tested in this update.
The runtime and bridge bytes are identical to Alpha 0.4.0-alpha.2.
