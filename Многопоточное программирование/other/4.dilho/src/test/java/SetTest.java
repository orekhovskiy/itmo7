import org.junit.Test;

import java.util.HashSet;
import java.util.Random;


import static org.junit.Assert.*;

public class SetTest {

    @Test
    public void isEmptyTest() {
        Set<String> set = new SetImpl<>();
        assertEquals(true, set.isEmpty());
    }

    @Test
    public void isNotEmptyTest() {
        Set<String> set = new SetImpl<>();
        set.add("test");
        assertEquals(false, set.isEmpty());
    }

    @Test
    public void containsTest() {
        String value = "test";
        Set<String> set = new SetImpl<>();
        set.add(value);
        assertEquals(true, set.contains(value));
    }

    @Test
    public void notContainsTest() {
        String value = "test";
        String mockValue = "";
        Set<String> set = new SetImpl<>();
        set.add(mockValue);
        assertEquals(false, set.contains(value));
    }

    @Test
    public void addTest() {
        String value = "test";
        Set<String> set = new SetImpl<>();
        assertEquals(true, set.add(value));
        assertEquals(true, set.contains(value));
    }

    @Test
    public void rejectTest() {
        String value = "test";
        Set<String> set = new SetImpl<>();
        assertEquals(true, set.add(value));
        assertEquals(true, set.contains(value));
        assertEquals(false, set.add(value));
    }

    @Test
    public void removeTest() {
        String value = "test";
        Set<String> set = new SetImpl<>();
        set.add(value);
        assertEquals(true, set.remove(value));
        assertEquals(false, set.contains(value));
    }

    @Test
    public void doNotRemoveTest() {
        String value = "test";
        Set<String> set = new SetImpl<>();
        assertEquals(false, set.remove(value));
        assertEquals(false, set.contains(value));
    }

    @Test
    public void setPropertyTest() {
        java.util.Set<Integer> set = new HashSet<>();
        Set<Integer> testingSet = new SetImpl<>();
        int dataCount = 433;
        int bound = 10;
        Random random = new Random();
        for (int i = 0; i < dataCount; i++) {
            int value = random.nextInt(bound);
            assertEquals(set.add(value), testingSet.add(value));
            assertEquals(true, testingSet.contains(value));
            assertEquals(false, testingSet.isEmpty());
        }

        for (Integer value : set) {
            assertEquals(true, testingSet.contains(value));
            assertEquals(true, testingSet.remove(value));
            assertEquals(false, testingSet.contains(value));
        }
        assertEquals(true, testingSet.isEmpty());
    }
}