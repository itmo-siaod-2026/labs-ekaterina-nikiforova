import json
import os
import matplotlib.pyplot as plt

JSON_FILE = 'benchmark_results.json'

if not os.path.exists(JSON_FILE):
    print(f"Ошибка: Файл '{JSON_FILE}' не найден")
    exit(1)

with open(JSON_FILE, 'r') as f:
    data = json.load(f)

results = {
    1: {'get': {}, 'put': {}, 'merge': {}},
    4: {'get': {}, 'put': {}, 'merge': {}}
}

map_names = {
    'SimpleMap': 'SimpleHashMap\n(Non-thread-safe)',
    'CustomConcurrentMap': 'MyConcurrentHashMap\n(Custom)',
    'JDKConcurrentMap': 'ConcurrentHashMap\n(JDK)'
}

for run in data:
    full_name = run['benchmark'].split('.')[-1]  # Например, "get_1_SimpleMap"

    threads = run['threads']

    if 'get' in full_name:
        op_type = 'get'
    elif 'put' in full_name:
        op_type = 'put'
    elif 'merge' in full_name:
        op_type = 'merge'
    else:
        continue

    if 'SimpleMap' in full_name:
        display_name = 'SimpleHashMap\n(Non-thread-safe)'
    elif 'CustomConcurrentMap' in full_name:
        display_name = 'MyConcurrentHashMap\n(Custom)'
    elif 'JDKConcurrentMap' in full_name:
        display_name = 'ConcurrentHashMap\n(JDK)'
    else:
        continue

    score = run['primaryMetric']['score']
    error = run['primaryMetric']['scoreError']

    results[threads][op_type][display_name] = (score, error)

plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')

fig, axs = plt.subplots(2, 3, figsize=(18, 11))

colors_3 = ['#4caf50', '#ff9800', '#2196f3']  # Для 1-поточного режима (3 мапы)
colors_2 = ['#ff9800', '#2196f3']  # Для 4-поточного режима (только потокобезопасные мапы)

operations = ['get', 'put', 'merge']
op_titles = {
    'get': 'Чтение (get)',
    'put': 'Запись (put)',
    'merge': 'Слияние (merge)'
}

for col, op in enumerate(operations):
    ax = axs[0, col]
    op_data = results[1][op]

    labels = list(op_data.keys())
    values = [op_data[l][0] for l in labels]
    errors = [op_data[l][1] for l in labels]

    bars = ax.bar(labels, values, yerr=errors, capsize=8, color=colors_3, edgecolor='black', alpha=0.85)
    ax.set_title(f'{op_titles[op]} — 1 поток', fontsize=12, fontweight='bold', pad=10)
    ax.set_ylabel('Время выполнения (нс / опер)', fontsize=10)
    ax.grid(True, axis='y', linestyle='--', alpha=0.7)

    for bar in bars:
        yval = bar.get_height()
        ax.text(bar.get_x() + bar.get_width() / 2.0, yval + (max(values) * 0.02), f'{yval:.1f} ns', ha='center',
                va='bottom', fontsize=9, fontweight='bold')

for col, op in enumerate(operations):
    ax = axs[1, col]
    op_data = results[4][op]

    labels = [l for l in op_data.keys() if 'SimpleHashMap' not in l]
    values = [op_data[l][0] for l in labels]
    errors = [op_data[l][1] for l in labels]

    bars = ax.bar(labels, values, yerr=errors, capsize=8, color=colors_2, edgecolor='black', alpha=0.85)
    ax.set_title(f'{op_titles[op]} — 4 потока (конкурентно)', fontsize=12, fontweight='bold', pad=10)
    ax.set_ylabel('Время выполнения (нс / опер)', fontsize=10)
    ax.grid(True, axis='y', linestyle='--', alpha=0.7)

    for bar in bars:
        yval = bar.get_height()
        ax.text(bar.get_x() + bar.get_width() / 2.0, yval + (max(values) * 0.02), f'{yval:.1f} ns', ha='center',
                va='bottom', fontsize=9, fontweight='bold')

plt.suptitle('Сравнение производительности хеш-таблиц (Average Time, меньше — лучше)', fontsize=16, fontweight='bold',
             y=0.99)
plt.tight_layout()

save_path = '../report/benchmark_results.png'
if not os.path.exists('../report'):
    save_path = 'benchmark_results.png'

plt.savefig(save_path, dpi=300)
plt.show()
