import ollama
import time

READING_SPEED_WORDS_PER_MIN = 300

def get_ollama_response(model, messages):
    """Call Ollama to get a response"""
    response = ollama.chat(model=model, messages=messages)
    return response['message']['content']

def calculate_reading_time(text):
    """Calculate reading time based on word count (in seconds)"""
    word_count = len(text)
    minutes = word_count / READING_SPEED_WORDS_PER_MIN
    return max(int(minutes * 60), 2)  # Wait at least 2 seconds

def main():
    model_name = "gemma3:4b"  # Please modify according to the model name installed locally
    conversation_history = []

    # Step 1 & 3: Define your sequence of questions
    questions = [
        "Hello, who are you?",
        "What is the date of the most recent data you have access to?",
        "How does memory management in XNU differ from Linux?",
        "Why didn't Linux adopt XNU's memory management?",
        "Who benefits the most from UMA (Unified Memory Architecture)?",
        "Have any non-Apple employees ever made real contributions to the XNU kernel?",
        "Among BSD (the three branches), Linux, and XNU, which project has the most volunteer contributions?",
    ]

    for question in questions:
        print(f"\n[OLLAMA TEST] Question: {question}")

        # Add user question to history
        conversation_history.append({'role': 'user', 'content': question})

        # Get response
        answer = get_ollama_response(model_name, conversation_history)
        print(f"\nAI Reply:\n{answer}")

        # Save AI response to history
        conversation_history.append({'role': 'assistant', 'content': answer})

        # Step 2: Calculate time and wait
        wait_time = calculate_reading_time(answer)
        print(f"\n[OLLAMA TEST] Estimated reading time: {wait_time} seconds")
        time.sleep(wait_time)

    print("[OLLAMA TEST] OK")

if __name__ == "__main__":
    main()
