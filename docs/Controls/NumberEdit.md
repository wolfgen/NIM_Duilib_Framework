# NumberEdit（数字文本框控件）

`NumberEdit` 继承了 `RichEdit` 的属性和方法，更多可用属性和方法请参考：[RichEdit](RichEdit.md) 控件

## 可用属性

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| intonly | false | BOOL | 是否仅支持输入整数,如(true) |
| allownegative | false | BOOL | 是否允许输入负数,如(true) |

## 可用接口

| 方法名称 | 用途 |
| :--- | :--- |
| [IsIntOnly](#IsIntOnly) | 判断是否仅支持输入整数 |
| [SetIntOnly](#SetIntOnly) | 设置是否仅支持输入整数 |
| [IsAllowNegative](#IsAllowNegative) | 判断是否允许输入负数 |
| [SetAllowNegative](#SetAllowNegative) | 设置是否允许输入负数 |

### IsIntOnly

判断是否仅支持输入整数

```cpp
bool IsIntOnly()
```

 - 参&emsp;数：无  
 - 返回值：返回 true 表示只能输入整数，false 表示可以整数也可以实数

### SetIntOnly

判断是否仅支持输入整数

```cpp
void SetIntOnly(bool bIntOnly = true)
```

 - 参&emsp;数：  
    - `bIntOnly` 为 true 则限制只能输入整数，false 为不限制
 - 返回值：无

### IsAllowNegative

判断是否允许输入负数

```cpp
bool IsAllowNegative()
```

 - 参&emsp;数：无  
 - 返回值：返回 true 表示允许输入负数数值，false 表示只能是正数

### SetAllowNegative

设置是否允许输入负数

```cpp
void SetAllowNegative(bool bAllowNegative = true)
```

 - 参&emsp;数：  
    - `bAllowNegative` 为 true 则允许进行负数输入，false 为不允许
 - 返回值：无



