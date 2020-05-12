# 一个编译原理练习程序

## 目前支持：
* 变量声明
* if else、while、do while结构语句及嵌套
* 自增操作
* 比较、逻辑、加减乘除运算表达式及组合
* read 和 print

## 示例：猜数字游戏脚本
启动ScriptC,输入脚本test.txt的路径回车执行。数字比预期数字小提示“too small”,比预期数字大提示“too large”，猜中则游戏结束。

test.txt内容:
```$xslt
int i,target=55;//还不支持数组和Random
do
{
    print "Guess a number and press enter:";
    read i;
    if(i>target)
        print "Too large!";
    else if(i<target)
        print "Too small!";
    else
    {
        print "You win.The number is:";
        print i;
    }
}while(i!=target);

```
执行效果：
```$xslt
Please input the script path:
C:\Users\Yorkin\prj\cpp\ScriptC\test.txt
Guess a number and press enter:
100
Too large!
Guess a number and press enter:
40
Too small!
Guess a number and press enter:
55
You win.The number is:
55
```
