import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
from tabulate import tabulate
import os


def process_perfect_hash_benchmark(csv_path):
    try:
        df = pd.read_csv(csv_path)
    except FileNotFoundError:
        print(f"Файл {csv_path} не найден.")
        return

    os.makedirs('report/data', exist_ok=True)

    operations = ['BUILD', 'GET']
    sns.set_theme(style="whitegrid")

    df_melted = df.melt(id_vars=['N', 'Iteration'], value_vars=operations,
                        var_name='Operation', value_name='Latency_ns')

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))

    sns.lineplot(data=df_melted[df_melted['Operation'] == 'BUILD'],
                 x='N', y='Latency_ns', color='teal',
                 marker='s', errorbar=('ci', 95), ax=ax1)
    ax1.set_title('BUILD', fontsize=14, fontweight='bold')
    ax1.set_ylabel('Latency (ns/key)')

    sns.lineplot(data=df_melted[df_melted['Operation'] == 'GET'],
                 x='N', y='Latency_ns', color='darkorange',
                 marker='D', errorbar=('ci', 95), ax=ax2)
    ax2.set_title('GET', fontsize=14, fontweight='bold')
    ax2.set_ylabel('Latency (ns/op)')

    plt.tight_layout()
    plt.savefig('report/data/perfect_hash_latency_plot.png', dpi=300)
    plt.show()

    stats = df_melted.groupby(['N', 'Operation'])['Latency_ns'].agg(['mean', 'std', 'count']).reset_index()
    stats['ci'] = 1.96 * (stats['std'] / np.sqrt(stats['count']))
    stats['display'] = stats.apply(lambda r: f"{r['mean']:>8.2f} ± {r['ci']:<6.2f}", axis=1)

    pivot = stats.pivot(index='N', columns='Operation', values='display')
    pivot = pivot[operations]

    table_text = tabulate(pivot, headers='keys', tablefmt='psql', stralign='center')
    header = f"=== PERFECT HASH LATENCY RESULTS (mean ± 95% CI, ns/op) ===\n"
    final_output = header + table_text

    print("\n" + final_output)

    with open("report/data/perfect_hash_latency_table.txt", "w", encoding="utf-8") as f:
        f.write(final_output)


if __name__ == "__main__":
    process_perfect_hash_benchmark('tmp/perfect_hash_latency_raw.csv')
