# https://blog.csdn.net/qq_41775769/article/details/113527799


class Fruit:
    # 构造方法
    def __init__(self, name, color):
        self.name = name
        self.color = color

    # 类的普通方法
    def buy(self, price, num):
        print("水果的价格是：", price * num)


"""
    hasattr(object,'attrName'):判断该对象是否有指定名字的属性或方法，返回值是bool类型
    setattr(object,'attrName',value):给指定的对象添加属性以及属性值
    getattr(object,'attrName'):获取对象指定名称的属性或方法，返回值是str类型
    delattr(object,'attrName'):删除对象指定名称的属性或方法值，无返回值
"""
apple = Fruit("苹果", "红色")
print(hasattr(apple, "name"))  # 判断对象是否有该属性或方法
print(hasattr(apple, "buy"))

# 获取对象指定的属性值
print(getattr(apple, "name"))
print(apple.name)
f = getattr(apple, "buy")
f(5, 10)

# 设置对象对应的属性
setattr(apple, "weight", 100)

# 删除对象对应的属性
delattr(apple, "name")
print(hasattr(apple, "name"))
