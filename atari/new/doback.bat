ls -t | egrep ".c|.s|makefile|changes|backup" > files
echo Transmitting files:
cat files
dokermit files
touch backup
