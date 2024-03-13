"""
    judge.py usage:
    
    1. put judge.py with the "service.c" in the same directory
    2. compile "sercive.c" to "service" which is an ELF file
    3. The directory "sample-testcases" must in the same directory with judge.py
    4. copy my "outputTransform.py" to the same directory with judge.py
    
    ex: ( if the main directory is /prog2 )
      /prog2
        |---- /prog2/sample-testcases
        |---- /prog2/service
        |---- /prog2/judge.py
        |---- /prog2/outputTransform.py ( must copy by yourself, not in the original github resource )
        |---- /prog2/service.c
        |---- /prog2/util.h
    
    run:
        $ python3 judge.py 2>/dev/null 
        ( stderr is up to you )
        
    note:
        If you want to show the output of your program,
        just "ctrl+/" the print part in the function "strcmp" .
        
"""
from argparse import ArgumentParser
import subprocess
import time
import sys
import os
import tempfile

# 把輸出的pid跟secret轉成固定字串的腳本路徑
outputTransform = 'outputTransform.py'

### 輸出格式
def bold(str_):
    print("\33[1m" + str_ + "\33[0m")

def red(str_):
    print("\33[31m" + str_ + "\33[0m")

def green(str_):
    print("\33[32m" + str_ + "\33[0m")


### 有用的
def Transform(path):
    result = subprocess.run(['python3', outputTransform, path], stdout=subprocess.PIPE, text=True)
    # 檢查命令的退出狀態碼（0 表示成功，非零表示失敗）
    if result.returncode == 0:
        print("Transform executed successfully.", file=sys.stderr)
        output = result.stdout
        return output
    else:
        print("Transform failed with return code:", result.returncode, file=sys.stderr)


def Execute(input_path):
    try:
        with open(input_path, 'r') as file:
            input_data = file.read()
        
        proc = subprocess.run(['./service', 'Manager'], input=input_data, stdout=subprocess.PIPE, text=True, timeout=5)
        
        if proc.returncode == 0:
            print("./service executed successfully.",file=sys.stderr)
            return proc.stdout
        else:
            print("./service failed with return code:", proc.returncode, file=sys.stderr)
            return None
    except subprocess.TimeoutExpired:
        print("超時5秒 你死了")
        return None  
    except Exception as e:
        print("錯誤：", str(e))
        return None   
    
    
def strcmp(input_path, ans_path):
    with tempfile.NamedTemporaryFile(delete=False) as file:
        file.write(Execute(input_path).encode('utf-8'))
        file.seek(0)
        result = Transform(file.name)
    # print("你的輸出:\n",result)
    # print("參考答案:\n",Transform(ans_path))
    return result == Transform(ans_path)


Input = []
Answer = []
for i in range(1,6):
    Input.append(f"sample-testcases/sample-input-{i}.txt")
    Answer.append(f"sample-testcases/sample-output-{i}.txt")

round = 1
for inp, ans in zip(Input, Answer):
    bold(f"===== Testcase {round} =====")
    try:
        assert strcmp(inp, ans)
        green(f"Testcase {round}: passed")
    except Exception as e:
        red(f"Testcase {round}: failed")
        print("錯誤：", str(e))
    round += 1
    
    
    
