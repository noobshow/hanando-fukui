# testfunccall.sh

clang foo.c -o foo.o -c
naiyou2="int main(){$1}"
#./hanando "$naiyou2" > tmp.s
./main "$naiyou2" > tmp.s
clang tmp.s foo.o -o tmp
actual=$(./tmp)
retval=$?
if [ $actual != $2 ]; then
   echo "Error: $2 but $actual"
   exit 1
fi
if [ $retval != $3 ]; then
   echo "Error: Retren $3 but $retval"
   exit 1
fi
