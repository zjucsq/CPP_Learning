# https://zhuanlan.zhihu.com/p/373340063

import asyncio


async def func1(i):
    print(f"协程函数{i}马上开始执行。")
    await asyncio.sleep(2)
    return i


async def main1():
    tasks = []
    for i in range(1, 5):
        tasks.append(asyncio.create_task(func1(i)))

    # 获取任务执行结果。
    done, pending = await asyncio.wait(tasks)
    for task in done:
        print(f"执行结果: {task.result()}")


async def func2(i):
    print(f"协程函数{i}马上开始执行。")
    await asyncio.sleep(2)
    return i


async def main2():
    tasks = []
    for i in range(1, 5):
        tasks.append(func1(i))

    results = await asyncio.gather(*tasks)
    for result in results:
        print(f"执行结果: {result}")


if __name__ == "__main__":
    asyncio.run(main1())
    asyncio.run(main2())
