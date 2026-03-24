import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
from tabulate import tabulate
import os


def process_benchmark(csv_path):
    try:
        df = pd.read_csv(csv_path)
    except FileNotFoundError:
        print(f"Файл {csv_path} не найден.")
        return

    os.makedirs('report/data', exist_ok=True)

    operations = ['INSERT', 'UPDATE', 'DELETE', 'GET']
    sns.set_theme(style="whitegrid")

    # Подготовка данных
    df_melted = df.melt(id_vars=['N', 'Iteration'], value_vars=operations,
                        var_name='Operation', value_name='Latency_ns')

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 12))

    sns.lineplot(data=df_melted[df_melted['Operation'] == 'INSERT'],
                 x='N', y='Latency_ns', color='crimson',
                 marker='o', errorbar=('ci', 95), ax=ax1)
    ax1.set_title('Latency: INSERT', fontsize=14, fontweight='bold')
    ax1.set_xscale('log')
    ax1.set_ylabel('Latency (ns/op)')

    others = ['UPDATE', 'DELETE', 'GET']
    sns.lineplot(data=df_melted[df_melted['Operation'].isin(others)],
                 x='N', y='Latency_ns', hue='Operation',
                 marker='o', errorbar=('ci', 95), ax=ax2)
    ax2.set_title('Latency: UPDATE, DELETE, GET', fontsize=14, fontweight='bold')
    ax2.set_xscale('log')
    ax2.set_ylabel('Latency (ns/op)')

    plt.tight_layout()
    plt.savefig('report/data/extendible_latency_plot.png', dpi=300)
    plt.show()

    stats = df_melted.groupby(['N', 'Operation'])['Latency_ns'].agg(['mean', 'std', 'count']).reset_index()
    stats['ci'] = 1.96 * (stats['std'] / np.sqrt(stats['count']))

    stats['display'] = stats.apply(lambda r: f"{r['mean']:>8.2f} ± {r['ci']:<6.2f}", axis=1)

    pivot = stats.pivot(index='N', columns='Operation', values='display')
    pivot = pivot[operations]

    table_text = tabulate(pivot, headers='keys', tablefmt='psql', stralign='center')
    header = f"=== РЕЗУЛЬТАТЫ ЗАДЕРЖКИ (среднее ± 95% CI, ns/op) ===\n"
    final_output = header + table_text

    print("\n" + final_output)

    with open("report/data/extendible_latency_table.txt", "w", encoding="utf-8") as f:
        f.write(final_output)


if __name__ == "__main__":
    process_benchmark('tmp/extendible_latency_raw.csv')
