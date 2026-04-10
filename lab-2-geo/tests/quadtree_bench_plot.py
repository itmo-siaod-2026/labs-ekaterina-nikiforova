import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
from tabulate import tabulate


def process_quadtree_benchmark(csv_path):
    try:
        df = pd.read_csv(csv_path)
    except FileNotFoundError:
        print(f"Файл {csv_path} не найден. Сначала запустите C++ бенчмарк.")
        return

    operations = ['INSERT', 'GET']
    sns.set_theme(style="whitegrid")

    df_melted = df.melt(id_vars=['N', 'Iteration'], value_vars=operations,
                        var_name='Operation', value_name='Latency_ns')

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 12))

    sns.lineplot(data=df_melted[df_melted['Operation'] == 'INSERT'],
                 x='N', y='Latency_ns', color='crimson',
                 marker='o', errorbar=('ci', 95), ax=ax1)
    ax1.set_title('Quadtree Latency: INSERT (Point Insertion)', fontsize=14, fontweight='bold')
    ax1.set_ylabel('Latency (ns/op)')
    ax1.set_xlabel('Number of Points (N)')

    sns.lineplot(data=df_melted[df_melted['Operation'] == 'GET'],
                 x='N', y='Latency_ns', color='teal',
                 marker='s', errorbar=('ci', 95), ax=ax2)
    ax2.set_title('Quadtree Latency: GET (Spatial Query Range)', fontsize=14, fontweight='bold')
    ax2.set_ylabel('Latency (ns/query)')
    ax2.set_xlabel('Number of Points (N)')

    plt.tight_layout()
    plt.savefig('lab-2-geo/report/data/quadtree_latency_plot.png', dpi=300)
    print("[OK] График сохранен: lab-2-geo/report/data/quadtree_latency_plot.png")
    plt.show()

    # Формирование таблицы результатов
    stats = df_melted.groupby(['N', 'Operation'])['Latency_ns'].agg(['mean', 'std', 'count']).reset_index()

    # Считаем доверительный интервал 95%
    stats['ci'] = 1.96 * (stats['std'] / np.sqrt(stats['count']))
    stats['display'] = stats.apply(lambda r: f"{r['mean']:>8.2f} ± {r['ci']:<6.2f}", axis=1)

    pivot = stats.pivot(index='N', columns='Operation', values='display')

    pivot = pivot[operations]

    table_text = tabulate(pivot, headers='keys', tablefmt='psql', stralign='center')
    header = f"=== РЕЗУЛЬТАТЫ QUADTREE (среднее ± 95% CI, ns/op) ===\n"
    final_output = header + table_text

    print("\n" + final_output)

    with open("lab-2-geo/report/data/quadtree_latency_table.txt", "w", encoding="utf-8") as f:
        f.write(final_output)
    print("[OK] Таблица сохранена: lab-2-geo/report/data/quadtree_latency_table.txt")


if __name__ == "__main__":
    process_quadtree_benchmark('tmp/quadtree_latency_raw.csv')
