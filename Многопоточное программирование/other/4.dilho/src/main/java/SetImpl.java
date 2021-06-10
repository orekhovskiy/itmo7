import java.util.concurrent.atomic.AtomicMarkableReference;

public class SetImpl<T extends Comparable<T>> implements Set<T> {

    private final Node<T> head = new Node<>(null, null);

    private static class Node<T> {
        final T value;
        final AtomicMarkableReference<Node<T>> nextReference;
        Node(T value, Node<T> next) {
            this.value = value;
            nextReference = new AtomicMarkableReference<>(next, false);
        }
    }

    @Override
    public boolean add(T value) {
        while (true) {
            Node<T>[] nodes = find(value);
            if (nodes[1] != null && nodes[1].value.equals(value)) {
                return false;
            }
            Node<T> tail = new Node<>(value, nodes[1]);
            if (nodes[0].nextReference.compareAndSet(nodes[1], tail, false, false)) {
                return true;
            }
        }
    }

    @Override
    public boolean remove(T value) {
        while (true) {
            Node<T>[] nodes = find(value);

            if (nodes[1] == null || nodes[1].value.compareTo(value) != 0) {
                return false;
            }

            Node<T> tail = nodes[1].nextReference.getReference();
            if (!nodes[1].nextReference.attemptMark(tail, true)) {
                continue;
            }
            nodes[0].nextReference.compareAndSet(nodes[1], tail, false, false);
            return true;
        }
    }

    @Override
    public boolean contains(T value) {
        Node<T> found = head.nextReference.getReference();

        while (found != null && found.value.compareTo(value) < 0) {
            found = found.nextReference.getReference();
        }

        return found != null && found.value.compareTo(value) == 0 && !found.nextReference.isMarked();
    }

    @Override
    public boolean isEmpty() {
        Node<T> found = head.nextReference.getReference();

        while (found != null) {
            if(found.nextReference.isMarked())
                found = found.nextReference.getReference();
            else
                return false;
        }

        return true;
    }

    private Node<T>[] find(T value) {
        retry:
        while (true) {
            Node<T> prev = head;
            Node<T> curr = head.nextReference.getReference();
            Node<T> tmp;
            while (curr != null) {
                tmp = curr.nextReference.getReference();
                if (curr.nextReference.isMarked()) {
                    if (!prev.nextReference.compareAndSet(curr, tmp, false, false)) {
                        continue retry;
                    }
                } else {
                    if (curr.value.compareTo(value) >= 0) {
                        return new Node[] {prev, curr};
                    }
                    prev = curr;
                }
                curr = tmp;
            }
            return new Node[] {prev, curr};
        }
    }
}