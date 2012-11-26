/**
 * @author : Li Yu
 * @date : 10/27/2012
 */
package com.javautil.cache;

import java.util.Map;
import java.util.HashMap;
import java.util.Collections;

interface Cache<K, V>
{
    V get(K key);
    void put(K key, V value);
    void remove(K key);
    int size();
}

class Entry<K, V>
{
    public final K key;    // key should be immutable
    public V value;

    Entry(K key, V value)
    {
        this.key = key;
        this.value = value;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (obj instanceof Entry)
        {
            Entry entry = (Entry)obj;
            return key.equals(entry.key) && value.equals(entry.value);
        }
        else
            return false;
    }

    @Override
    public int hashCode()
    {
        return key.hashCode()^value.hashCode();
    }
}

class LRUCache<K, V> implements Cache<K, V>
{
    class Node
    {
        public K key;
        public V value;
        public long expires;
        public Node previous, next;

        public Node()
        {}

        public Node(K key, V value)
        {
            this.key = key;
            this.value = value;
        }
    }

    Map<K, Node> node_map = Collections.synchronizedMap(new HashMap<K, Node>());
    Node first = new Node(), last = new Node();
    int max_size;

    public LRUCache(int max_size)
    {
        this.max_size = max_size;
        first.next = last;
        last.previous = first;
    }

    synchronized void addToFirst(Node node)
    {
        node.previous = first;
        node.next = first.next;
        first.next.previous = node;
        first.next = node;
    }

    synchronized void moveToFirst(Node node)
    {
        node.previous.next = node.next;
        node.next.previous = node.previous;
        node.previous = first;
        node.next = first.next;
        first.next.previous = node;
        first.next = node;        
    }

    synchronized void remove(Node node)
    {
        node.previous.next = node.next;
        node.next.previous = node.previous;
        node.previous = null;
        node.next = null;        
    }

    public V get(K key)
    {
        Node node = node_map.get(key);
        if (node == null)
            return null;
        if (node != first.next)
            moveToFirst(node);
        return node.value;
    }

    public void put(K key, V value)
    {
        Node node = node_map.get(key);
        if (node == null)
        {
            node = new Node(key, value);
            node_map.put(key, node);
            addToFirst(node);
        }
        else
        {
            node.value = value;
            moveToFirst(node);
        }

        if (node_map.size() > max_size)
            remove(last.previous);
    }

    public void remove(K key)
    {
        Node node = node_map.get(key);
        if (node == null)
            return;
        remove(node);
        node_map.remove(key);
    }

    public int size()
    {
        return node_map.size();
    }

    public void print()
    {
        Node current = first;
        String chain = "";
        while (current != null)
        {
            chain += current.value + " ";
            current = current.next;
        }
        System.out.println(chain);
    }
}
