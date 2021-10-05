
#This script will revert the changes made by the kernel module

if [ $LOGNAME != "root" ]
then
	echo "Run this script as root"
else
	echo 3 > /proc/sys/vm/drop_caches
	echo "Page cache dumped"
fi
