#!/bin/bash
my_fun()
{
  echo "$#"
}
echo 'the number of parameter in "$@" is '$(my_fun "$@")
echo 'the number of parameter in "$*" is '$(my_fun "$*") 

echo 'number $*:' $*
echo "number:$@"
echo "number:$#"
echo "number:$0"
echo "number:$1"
echo "number:$2"
echo "number:$3"
echo "the number ': $#' "
echo $$
