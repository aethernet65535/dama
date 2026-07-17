import ollama
import time

# 假设阅读速度为每分钟 300 字
READING_SPEED_WORDS_PER_MIN = 300

def get_ollama_response(model, messages):
    """调用 Ollama 获取回复"""
    response = ollama.chat(model=model, messages=messages)
    return response['message']['content']

def calculate_reading_time(text):
    """根据字数计算阅读时间（单位：秒）"""
    word_count = len(text)
    minutes = word_count / READING_SPEED_WORDS_PER_MIN
    return max(int(minutes * 60), 2)  # 至少等待 2 秒

def main():
    model_name = "gemma3:4b"  # 请根据你本地安装的模型名称修改
    conversation_history = []
    
    # 步骤 1 & 3: 定义你的问题序列
    questions = [
        "您好，您是谁？",
        "请问您能获取到的最新数据是什么时间的？",
        "XNU 和 Linux 的内存管理有何不同？",
        "Linux 为什么没有照搬 XNU 的内存管理？",
        "UMA（统一内存架构）对什么人群最好？",
        "有非 Apple 员工向 XNU 内核真实的贡献过吗？",
        "请问 BSD（三个分支）、Linux 和 XNU，哪个项目的志愿者贡献最多？",
    ]
    
    for question in questions:
        print(f"\n--- Question: {question} ---")
        
        # 添加用户问题到历史
        conversation_history.append({'role': 'user', 'content': question})
        
        # 获取回复
        answer = get_ollama_response(model_name, conversation_history)
        print(f"\nAI Reply:\n{answer}")
        
        # 保存 AI 回复到历史
        conversation_history.append({'role': 'assistant', 'content': answer})
        
        # 步骤 2: 计算时间并等待
        wait_time = calculate_reading_time(answer)
        print(f"\n[SYSTEM] Estimated reading time: {wait_time} seconds")
        time.sleep(wait_time)

    print("[SYSTEM] OK")

if __name__ == "__main__":
    main()
