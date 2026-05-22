package org.example;

import java.util.*;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.atomic.AtomicReferenceArray;
import java.util.concurrent.atomic.LongAdder;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.function.BiFunction;

public class MyConcurrentHashMap<K, V> implements Iterable<Map.Entry<K, V>> {

    private static final int DEFAULT_CAPACITY = 16;
    private static final int MAXIMUM_CAPACITY = 1 << 30;
    private static final double LOAD_FACTOR = 0.5;

    private volatile AtomicReferenceArray<Bucket<K, V>> table;
    private final LongAdder size = new LongAdder();
    private final ReentrantReadWriteLock tableLock = new ReentrantReadWriteLock();

    private static final class Bucket<K, V> {
        final K key;
        final AtomicReference<V> value;
        final Bucket<K, V> next;

        Bucket(K key, V value, Bucket<K, V> next) {
            this.key = key;
            this.value = new AtomicReference<>(value);
            this.next = next;
        }
    }

    public MyConcurrentHashMap() {
        this(DEFAULT_CAPACITY);
    }

    public MyConcurrentHashMap(int initialCapacity) {
        if (initialCapacity <= 0) {
            throw new IllegalArgumentException();
        }
        this.table = new AtomicReferenceArray<>(initialCapacity);
    }

    public V get(K key) {
        final int hash = calcHash(key);
        final AtomicReferenceArray<Bucket<K, V>> t = table;
        for (Bucket<K, V> e = t.get(bucketIndex(hash, t.length())); e != null; e = e.next) {
            if (Objects.equals(e.key, key)) return e.value.get();
        }
        return null;
    }

    public V put(K key, V value) {
        final int hash = calcHash(key);
        V prev = null;

        final Lock readLock = tableLock.readLock();
        readLock.lock();
        try {
            final AtomicReferenceArray<Bucket<K, V>> t = table;
            final int i = bucketIndex(hash, t.length());
            while (true) {
                final Bucket<K, V> head = t.get(i);
                boolean updated = false;
                for (Bucket<K, V> e = head; e != null; e = e.next) {
                    if (Objects.equals(e.key, key)) {
                        do {
                            prev = e.value.get();
                        } while (!e.value.compareAndSet(prev, value));
                        updated = true;
                        break;
                    }
                }
                if (updated) {
                    break;
                }
                if (t.compareAndSet(i, head, new Bucket<>(key, value, head))) {
                    size.increment();
                    break;
                }
            }
        } finally {
            readLock.unlock();
        }

        resize();

        return prev;
    }

    public V merge(K key, V value, BiFunction<? super V, ? super V, ? extends V> remappingFunction) {
        final int hash = calcHash(key);
        V resultValue = null;

        final Lock readLock = tableLock.readLock();
        readLock.lock();
        try {
            final AtomicReferenceArray<Bucket<K, V>> t = table;
            final int i = bucketIndex(hash, t.length());
            while (true) {
                final Bucket<K, V> head = t.get(i);
                boolean updated = false;
                for (Bucket<K, V> e = head; e != null; e = e.next) {
                    if (Objects.equals(e.key, key)) {
                        V prev;
                        do {
                            prev = e.value.get();
                            resultValue = remappingFunction.apply(prev, value);
                        } while (!e.value.compareAndSet(prev, resultValue));
                        updated = true;
                        break;
                    }
                }
                if (updated) {
                    break;
                }
                if (t.compareAndSet(i, head, new Bucket<>(key, value, head))) {
                    size.increment();
                    resultValue = value;
                    break;
                }
            }
        } finally {
            readLock.unlock();
        }

        resize();

        return resultValue;
    }

    public int size() {
        return (int) size.sum();
    }

    public void clear() {
        final Lock writeLock = tableLock.writeLock();
        writeLock.lock();
        try {
            table = new AtomicReferenceArray<>(DEFAULT_CAPACITY);
            size.reset();
        } finally {
            writeLock.unlock();
        }
    }

    private void resize() {
        final Lock writeLock = tableLock.writeLock();
        writeLock.lock();
        try {
            if (size.sum() < (long) (table.length() * LOAD_FACTOR)) {
                return;
            }

            final AtomicReferenceArray<Bucket<K, V>> old = table;
            final int oldLength = old.length();
            if (oldLength >= MAXIMUM_CAPACITY) {
                return;
            }

            final int newCap = oldLength * 2;
            final AtomicReferenceArray<Bucket<K, V>> grown = new AtomicReferenceArray<>(newCap);

            for (int i = 0; i < oldLength; i++) {
                for (Bucket<K, V> e = old.get(i); e != null; e = e.next) {
                    final int hash = calcHash(e.key);
                    final int ni = bucketIndex(hash, newCap);
                    grown.set(ni, new Bucket<>(e.key, e.value.get(), grown.get(ni)));
                }
            }
            table = grown;
        } finally {
            writeLock.unlock();
        }
    }

    private static int calcHash(Object key) {
        if (key == null) return 0;
        final int h = key.hashCode();
        return h ^ (h >>> 16);
    }

    private static int bucketIndex(int hash, int tableLength) {
        return (hash & 0x7fffffff) % tableLength;
    }

    @Override
    public Iterator<Map.Entry<K, V>> iterator() {
        return new MapIterator(table);
    }

    private final class MapIterator implements Iterator<Map.Entry<K, V>> {
        private final AtomicReferenceArray<Bucket<K, V>> snapshot;
        private int bucketIndex;
        private Bucket<K, V> next;

        MapIterator(AtomicReferenceArray<Bucket<K, V>> snapshot) {
            this.snapshot = snapshot;
            advance();
        }

        private void advance() {
            while (next == null && bucketIndex < snapshot.length()) next = snapshot.get(bucketIndex++);
        }

        @Override
        public boolean hasNext() {
            return next != null;
        }

        @Override
        public Map.Entry<K, V> next() {
            if (next == null) throw new NoSuchElementException();
            final Bucket<K, V> e = next;
            next = e.next;
            if (next == null) advance();
            return new AbstractMap.SimpleImmutableEntry<>(e.key, e.value.get());
        }
    }
}