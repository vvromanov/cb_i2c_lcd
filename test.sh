LINE1="Line 1"
LINE2="Line 2"
LINE3="Line 3"
LINE4="Line 4"
./i2c_lcd
for (( i=1; i<=10000; i++ )); do
    LINE2=`date +"%T %D"`
   ./i2c_lcd "Counter ${i}" "${LINE2}" "${LINE3}" "${LINE4}"
done