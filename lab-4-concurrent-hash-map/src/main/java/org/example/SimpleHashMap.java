package org.example;

import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Objects;
import java.util.function.BiFunction;

public class SimpleHashMap<K, V> implements Iterable<SimpleHashMap.Bucket<K, V>> {

    public static class Bucket<K, V> {
        final K key;
        V value;
        Bucket<K, V> next;

        public Bucket(K key, V value, Bucket<K, V> next) {
            this.key = key;
            this.value = value;
            this.next = next;
        }

        public K getKey() {
            return key;
        }

        public V getValue() {
            return value;
        }
    }

    private static final int DEFAULT_INITIAL_CAPACITY = 16;
    private static final double LOAD_FACTOR = 0.75;

    private Bucket<K, V>[] table;
    private int size;

    @SuppressWarnings("unchecked")
    public SimpleHashMap() {
        this.table = (Bucket<K, V>[]) new Bucket[DEFAULT_INITIAL_CAPACITY];
        this.size = 0;
    }

    private int getBucketIndex(K key) {
        if (key == null) return 0;
        return (key.hashCode() & 0x7fffffff) % table.length;
    }

    public void put(K key, V value) {
        if (size >= table.length * LOAD_FACTOR) {
            resize();
        }

        int index = getBucketIndex(key);
        Bucket<K, V> head = table[index];
        Bucket<K, V> current = head;

        while (current != null) {
            if (Objects.equals(current.key, key)) {
                current.value = value;
                return;
            }
            current = current.next;
        }

        table[index] = new Bucket<>(key, value, head);
        size++;
    }

    public V get(K key) {
        int index = getBucketIndex(key);
        Bucket<K, V> current = table[index];

        while (current != null) {
            if (Objects.equals(current.key, key)) {
                return current.value;
            }
            current = current.next;
        }
        return null;
    }

    public int size() {
        return size;
    }

    public void clear() {
        for (int i = 0; i < table.length; i++) {
            table[i] = null;
        }
        size = 0;
    }

    public V merge(K key, V value, BiFunction<? super V, ? super V, ? extends V> merger) {
        int index = getBucketIndex(key);
        Bucket<K, V> head = table[index];
        Bucket<K, V> current = head;

        while (current != null) {
            if (Objects.equals(current.key, key)) {
                V oldValue = current.value;
                V newValue = merger.apply(oldValue, value);
                current.value = newValue;
                return newValue;
            }
            current = current.next;
        }

        if (size >= table.length * LOAD_FACTOR) {
            resize();
            index = getBucketIndex(key);
            head = table[index];
        }
        table[index] = new Bucket<>(key, value, head);
        size++;
        return value;
    }

    @SuppressWarnings("unchecked")
    private void resize() {
        Bucket<K, V>[] oldTable = table;
        table = (Bucket<K, V>[]) new Bucket[oldTable.length * 2];
        for (Bucket<K, V> head : oldTable) {
            Bucket<K, V> current = head;
            while (current != null) {
                Bucket<K, V> next = current.next;
                int newIndex = getBucketIndex(current.key);

                current.next = table[newIndex];
                table[newIndex] = current;

                current = next;
            }
        }
    }

    @Override
    public Iterator<Bucket<K, V>> iterator() {
        return new Iterator<Bucket<K, V>>() {
            private int currentArrayIndex = 0;
            private Bucket<K, V> currentBucket = null;

            private void advance() {
                if (currentBucket != null && currentBucket.next != null) {
                    currentBucket = currentBucket.next;
                    return;
                }
                currentBucket = null;
                while (currentArrayIndex < table.length) {
                    if (table[currentArrayIndex] != null) {
                        currentBucket = table[currentArrayIndex];
                        currentArrayIndex++;
                        return;
                    }
                    currentArrayIndex++;
                }
            }

            {
                advance();
            }

            @Override
            public boolean hasNext() {
                return currentBucket != null;
            }

            @Override
            public Bucket<K, V> next() {
                if (!hasNext()) {
                    throw new NoSuchElementException();
                }
                Bucket<K, V> bucketToReturn = currentBucket;
                advance();
                return bucketToReturn;
            }
        };
    }
}