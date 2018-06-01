#!/bin/bash

directory=$1
text=$2
w=$3
p=$4

if [ -d $directory ] && [ -s $text ]
then
  echo "Directory and Text exist"
else
  echo "Directory and Text does not exist"
fi

empty=$( ls $directory )
if (( `expr length "empty"` > 0 ))
then
    echo "Warning:directory is full, purging"
    rm -rf $directory/*
fi

if ! [ "$w" -eq "$w" ] 2> /dev/null
then
    echo "W is not a number"
else
    echo "W is a number"
fi

if ! [ "$p" -eq "$p" ] 2> /dev/null
then
    echo "S is not a number"
else
    echo "S is a number"
fi

lines=$( wc -l <"$text" )
if [ $lines -lt 10000 ]
then
    echo "File's lines less than 10000"
    exit
else
    echo "File's lines greater than 10000"
fi

#TOUCH FILE
position=0
for ((i=0;i<$w;i++))
do
    echo "Creating Web Site $i ..."
    mkdir "$directory/site$i"
    for ((j=0;j<$p;j++))
    do
      rnum=$RANDOM
      touch "$directory/site$i/page$j""_$rnum.html"
      array[$position]="$directory/site$i/page$j""_$rnum.html"
      temp=${array[$position]}
      k=$(($RANDOM % ($lines-2002) + 2))
      m=$(($RANDOM % 999 + 1001))
      if [ $j -eq 0 ]
      then
        echo $temp
        echo "Creating page $temp with lines $m starting at line $k ..."
      fi
      echo "Adding link to $temp"
      echo "<!DOCTYPE html> <html> <body>" > $temp
      let " position = position + 1"
    done
done

position=0
for ((i=0;i<$w;i++))
do
    for ((j=0;j<$p;j++))
    do
      #k=$(($RANDOM % ($lines-2002) + 2))
      #m=$(($RANDOM % 999 + 1001))
      let f=p/2+1
      let q=w/2+1
      if [ $p -eq 1 ]
      then
        let f=0
      fi
      if [ $w -eq 1 ]
      then
        let q=0
      fi
      for ((z=0;z<$f;z++))
      do
        let fpage=i*p
        let cpage=i*p+j
        internal[z]=$(($RANDOM % $p + $fpage))
        while [ ${internal[z]} -eq $cpage ]; do
          internal[z]=$(($RANDOM % $p + $fpage))
        done
      done
      for ((n=0;n<$q;n++))
      do
        let fsite=i*p
        let tmp=w-1
        let extsite=tmp*p
        let lsite=fsite+p-1
        external[$n]=$(($RANDOM % $extsite))
        if [ ${external[$n]} -ge $fsite ]
        then
            if [ ${external[$n]} -le $lsite ]
            then
              let tmp=w-1-i
              let "external[n]=external[n]+p*tmp"
            fi
        fi
      done
      if [ $w -eq 1 ]
      then
          if [ $p -eq 1 ]
          then
            let f=1
            let q=1
          fi
      fi
      let tmp=f+q
      let clines=m/tmp #copied lines
      let tmp=1
      while [ $tmp -lt $k ]; do
          IFS='' read -r line done < $text
          let "tmp=tmp+1"
      done

      for (( a=0;a<$f;a++))
      do
          for ((b=0;b<$clines;b++))
          do
            IFS='' read -r line
            echo $line >> ${array[$position]}
          done<$text
          temp4=${internal[$a]}
          echo "<a href=../../${array[$temp4]}>link$a""_text</a>"
      done >> ${array[$position]}
      for (( a=0;a<$q;a++))
      do
          for ((b=0;b<$clines;b++))
          do
            IFS='' read -r line
            echo $line >> ${array[$position]}
          done<$text
          temp4=${external[$a]}
          echo "<a href=../../${array[$temp4]}>link$a""_text</a>"
      done >> ${array[$position]}
      let "position=position+1"
    done
done
