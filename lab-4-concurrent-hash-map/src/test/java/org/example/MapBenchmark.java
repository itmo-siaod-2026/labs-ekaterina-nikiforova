package org.example;

import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;
import org.openjdk.jmh.results.format.ResultFormatType;

import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@Warmup(iterations = 3, time = 2, timeUnit = TimeUnit.SECONDS)
@Measurement(iterations = 5, time = 2, timeUnit = TimeUnit.SECONDS)
@Fork(1)
public class MapBenchmark {

    @State(Scope.Benchmark)
    public static class MapState {
        public SimpleHashMap<Integer, Integer> simpleMap;
        public MyConcurrentHashMap<Integer, Integer> customConcurrentMap;
        public java.util.concurrent.ConcurrentHashMap<Integer, Integer> jdkConcurrentMap;

        @Setup(Level.Iteration)
        public void setUp() {
            simpleMap = new SimpleHashMap<>();
            customConcurrentMap = new MyConcurrentHashMap<>();
            jdkConcurrentMap = new java.util.concurrent.ConcurrentHashMap<>();

            for (int i = 0; i < 5000; i++) {
                simpleMap.put(i, i);
                customConcurrentMap.put(i, i);
                jdkConcurrentMap.put(i, i);
            }
        }
    }

    @State(Scope.Thread)
    public static class ThreadState {
        private int counter = 0;

        public int getNextKey() {
            counter = (counter + 1) & 0x1fff;
            return counter;
        }
    }

    // ==========================================
    // --- 1. ОДНОПОТОЧНЫЕ ТЕСТЫ (Threads = 1) ---
    // ==========================================

    @Benchmark
    @Threads(1)
    public Integer get_1_SimpleMap(MapState state, ThreadState threadState) {
        return state.simpleMap.get(threadState.getNextKey());
    }

    @Benchmark
    @Threads(1)
    public Integer get_2_CustomConcurrentMap(MapState state, ThreadState threadState) {
        return state.customConcurrentMap.get(threadState.getNextKey());
    }

    @Benchmark
    @Threads(1)
    public Integer get_3_JDKConcurrentMap(MapState state, ThreadState threadState) {
        return state.jdkConcurrentMap.get(threadState.getNextKey());
    }

    @Benchmark
    @Threads(1)
    public void put_1_SimpleMap(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        state.simpleMap.put(k, k);
    }

    @Benchmark
    @Threads(1)
    public void put_2_CustomConcurrentMap(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        state.customConcurrentMap.put(k, k);
    }

    @Benchmark
    @Threads(1)
    public void put_3_JDKConcurrentMap(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        state.jdkConcurrentMap.put(k, k);
    }

    @Benchmark
    @Threads(1)
    public Integer merge_1_SimpleMap(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        return state.simpleMap.merge(k, 1, Integer::sum);
    }

    @Benchmark
    @Threads(1)
    public Integer merge_2_CustomConcurrentMap(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        return state.customConcurrentMap.merge(k, 1, Integer::sum);
    }

    @Benchmark
    @Threads(1)
    public Integer merge_3_JDKConcurrentMap(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        return state.jdkConcurrentMap.merge(k, 1, Integer::sum);
    }

    // ===========================================
    // --- 2. МНОГОПОТОЧНЫЕ ТЕСТЫ (Threads = 4) ---
    // ===========================================

    @Benchmark
    @Threads(4)
    public Integer get_4_CustomConcurrentMap_Concurrent(MapState state, ThreadState threadState) {
        return state.customConcurrentMap.get(threadState.getNextKey());
    }

    @Benchmark
    @Threads(4)
    public Integer get_5_JDKConcurrentMap_Concurrent(MapState state, ThreadState threadState) {
        return state.jdkConcurrentMap.get(threadState.getNextKey());
    }

    @Benchmark
    @Threads(4)
    public void put_4_CustomConcurrentMap_Concurrent(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        state.customConcurrentMap.put(k, k);
    }

    @Benchmark
    @Threads(4)
    public void put_5_JDKConcurrentMap_Concurrent(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        state.jdkConcurrentMap.put(k, k);
    }

    @Benchmark
    @Threads(4)
    public Integer merge_4_CustomConcurrentMap_Concurrent(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        return state.customConcurrentMap.merge(k, 1, Integer::sum);
    }

    @Benchmark
    @Threads(4)
    public Integer merge_5_JDKConcurrentMap_Concurrent(MapState state, ThreadState threadState) {
        int k = threadState.getNextKey();
        return state.jdkConcurrentMap.merge(k, 1, Integer::sum);
    }

    public static void main(String[] args) throws Exception {
        Options opt = new OptionsBuilder()
                .include(MapBenchmark.class.getSimpleName())
                .resultFormat(ResultFormatType.JSON)
                .result("benchmark_results.json")
                .build();
        new Runner(opt).run();
    }
}