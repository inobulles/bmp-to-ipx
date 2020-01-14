
#!/bin/bash
set -e

echo "Compiling source ..."
gcc main.c -o bmp-to-ipx

echo "Moving binary to /bin ..."
sudo mv bmp-to-ipx /bin/bmp-to-ipx

exit 0
