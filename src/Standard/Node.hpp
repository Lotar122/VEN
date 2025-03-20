#pragma once

template<typename T>
struct Node
{
    Node<T>* prev = nullptr;
    Node<T>* next = nullptr;

    T* data = nullptr;
};