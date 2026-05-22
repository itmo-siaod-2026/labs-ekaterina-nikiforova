import json
import os
import matplotlib.pyplot as plt

JSON_FILE = 'benchmark_results.json'

if not os.path.exists(JSON_FILE):
    print(f"Ошибка: Файл '{JSON_FILE}' не найден")
    exit(1)

with open(JSON_FILE, 'r') as f:
    data = json.load(f)

get_scores = {}
get_errors = {}
put_scores = {}
put_errors = {}

map_names = {
    'SimpleMap': 'SimpleHashMap\n(Non-thread-safe)',
    'CustomConcurrentMap': 'MyConcurrentHashMap\n(Custom Concurrent)',
    'JDKConcurrentMap': 'ConcurrentHashMap\n(Standard JDK)'
}

for run in data:
    full_name = run['benchmark'].split('.')[-1]
    parts = full_name.split('_')
    op_type = parts[0]
    map_key = parts[2]

    score = run['primaryMetric']['score']
    error = run['primaryMetric']['scoreError']

    display_name = map_names.get(map_key, map_key)

    if op_type == 'get':
        get_scores[display_name] = score
        get_errors[display_name] = error
    elif op_type == 'put':
        put_scores[display_name] = score
        put_errors[display_name] = error

plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 6))

colors = ['#4caf50', '#ff9800', '#2196f3']

get_labels = list(get_scores.keys())
get_values = [get_scores[l] for l in get_labels]
get_errs = [get_errors[l] for l in get_labels]

bars1 = ax1.bar(get_labels, get_values, yerr=get_errs, capsize=8, color=colors, edgecolor='black', alpha=0.85)
ax1.set_title('Время чтения (get)', fontsize=14, fontweight='bold', pad=15)
ax1.set_ylabel('Среднее время выполнения (нс / операция)', fontsize=12)
ax1.grid(True, axis='y', linestyle='--', alpha=0.7)

for bar in bars1:
    yval = bar.get_height()
    ax1.text(bar.get_x() + bar.get_width() / 2.0, yval + (max(get_values) * 0.02), f'{yval:.2f} ns', ha='center',
             va='bottom', fontsize=10, fontweight='bold')

put_labels = list(put_scores.keys())
put_values = [put_scores[l] for l in put_labels]
put_errs = [put_errors[l] for l in put_labels]

bars2 = ax2.bar(put_labels, put_values, yerr=put_errs, capsize=8, color=colors, edgecolor='black', alpha=0.85)
ax2.set_title('Время записи (put)', fontsize=14, fontweight='bold', pad=15)
ax2.set_ylabel('Среднее время выполнения (нс / операция)', fontsize=12)
ax2.grid(True, axi
s='y', linestyle='--', alpha=0.7)

for bar in bars2:
    yval = bar.get_height()
    ax2.text(bar.get_x() + bar.get_width() / 2.0, yval + (max(put_values) * 0.02), f'{yval:.2f} ns', ha='center',
             va='bottom', fontsize=10, fontweight='bold')

plt.tight_layout()
plt.savefig('benchmark_results.png', dpi=300)
plt.show()
