package org.example;

import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

@JCStressTest
@Outcome(id = "2", expect = Expect.ACCEPTABLE)
@Outcome(id = "1", expect = Expect.ACCEPTABLE_INTERESTING)
@Outcome(id = "0", expect = Expect.ACCEPTABLE_INTERESTING)
@Outcome(id = "-1", expect = Expect.ACCEPTABLE_INTERESTING)
@State
public class SimpleHashMapStressTest {

    private final SimpleHashMap<Integer, Integer> map = new SimpleHashMap<>();

    @Actor
    public void actor1() {
        try {
            map.put(1, 100);
        } catch (Exception ignored) {
        }
    }

    @Actor
    public void actor2() {
        try {
            map.put(17, 200);
        } catch (Exception ignored) {
        }
    }

    @Arbiter
    public void arbiter(I_Result r) {
        try {
            boolean has1 = map.get(1) != null;
            boolean has17 = map.get(17) != null;

            if (has1 && has17) {
                r.r1 = 2;
            } else if (has1 || has17) {
                r.r1 = 1;
            } else {
                r.r1 = 0;
            }
        } catch (Exception e) {
            r.r1 = -1;
        }
    }
}