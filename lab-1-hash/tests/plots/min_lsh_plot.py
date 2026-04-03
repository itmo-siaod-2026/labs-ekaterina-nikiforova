import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
from tabulate import tabulate
import os
import io


def process_benchmark(csv_data):
    df = pd.read_csv(csv_data)

    os.makedirs('lab-1-hash/report/data', exist_ok=True)

    operations = ['ADD_ns', 'LSH_SEARCH_ns', 'FULL_SCAN_ns']
    sns.set_theme(style="whitegrid")

    df_melted = df.melt(id_vars=['N', 'Iteration'], value_vars=operations,
                        var_name='Operation', value_name='Latency_ns')

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))

    fast_ops = ['ADD_ns', 'LSH_SEARCH_ns']
    sns.lineplot(data=df_melted[df_melted['Operation'].isin(fast_ops)],
                 x='N', y='Latency_ns', hue='Operation', palette='viridis',
                 marker='o', errorbar=('ci', 95), ax=ax1)
    ax1.set_ylabel('ns/op')
    ax1.grid(True, which="both", ls="-", alpha=0.5)

    sns.lineplot(data=df_melted[df_melted['Operation'] == 'FULL_SCAN_ns'],
                 x='N', y='Latency_ns', color='crimson',
                 marker='s', errorbar=('ci', 95), ax=ax2)
    ax2.set_ylabel('ns/op')

    plt.tight_layout()
    plt.savefig('lab-1-hash/report/data/lsh_performance_plot.png', dpi=300)
    plt.show()

    stats = df_melted.groupby(['N', 'Operation'])['Latency_ns'].agg(['mean', 'std', 'count']).reset_index()
    stats['ci'] = 1.96 * (stats['std'] / np.sqrt(stats['count']))
    stats['display'] = stats.apply(lambda r: f"{r['mean']:>10.1f} ± {r['ci']:<8.1f}", axis=1)

    pivot = stats.pivot(index='N', columns='Operation', values='display')
    pivot = pivot[operations]

    table_text = tabulate(pivot, headers='keys', tablefmt='psql', stralign='center')
    header = f"=== LSH BENCHMARK RESULTS (mean ± 95% CI, ns/op) ===\n"
    final_output = header + table_text

    print("\n" + final_output)

    with open("lab-1-hash/report/data/lsh_latency_table.txt", "w", encoding="utf-8") as f:
        f.write(final_output)


if __name__ == "__main__":
    process_benchmark('tmp/min_hash_bench_raw.csv')
