#!/bin/bash

SOURCE_DIR=$1
TARGET_DIR=$2
PACKAGE_LIST=$3

usage() {
  echo "usage: ./symbol-link.sh <source packages directory> <target packages directory> <packages list>"
}

dbg() {
  echo $1
}

if [ "x$SOURCE_DIR" = "x" ] ; then
  usage
  exit -1
fi

if [ "x$TARGET_DIR" = "x" ] ; then
  usage
  exit -1
fi

if [ "x$PACKAGE_LIST" = "x" ] ; then
  usage
  exit -1
fi

while read LINE
do
  echo $LINE
  if [ "x$LINE" = "x" ] ; then
    echo "LINE is empty."
    continue;
  fi
  FIRST_CHAR=${LINE:0:1}
  if [ $FIRST_CHAR = "#" ] ; then
    echo "it's a comment"
  else
    SOURCE=${LINE%:*}
    TARGET=${LINE#*:}
    dbg "SOURCE=[$SOURCE]"
    dbg "TARGET=[$TARGET]"
    if [ "x$SOURCE" = "x" ] ; then
      echo "file format error @ [$LINE] !!!"
      exit -1
    fi
    if [ "x$TARGET" = "x" ] ; then
      echo "file format error @ [$LINE] !!!"
      exit -1
    fi
    if [ [ ! -d $SOURCE_DIR/$SOURCE ] -a [ ! -f $SOURCE_DIR/$SOURCE ] ] ; then
      echo "$SOURCE_DIR/$SOURCE does not exist !!!"
      exit -1
    fi
    dbg "mkdir -p $TARGET_DIR/$TARGET"
    mkdir -p $TARGET_DIR/$TARGET
    dbg "rm -rf $TARGET_DIR/$TARGET"
    rm -rf $TARGET_DIR/$TARGET
    dbg "ln -sf $SOURCE_DIR/$SOURCE $TARGET_DIR/$TARGET"
    ln -sf $SOURCE_DIR/$SOURCE $TARGET_DIR/$TARGET
  fi
done < $PACKAGE_LIST
