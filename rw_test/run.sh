ssd="/dev/nvme1n1"

sudo dmesg -C

rm rw

echo "compile"
g++ -std=c++11 rw_test.cc -o rw || exit

echo "run"
sudo ./rw $ssd

echo ""
echo ""
echo ""

dmesg
