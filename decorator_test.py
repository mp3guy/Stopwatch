from Stopwatch import Stopwatch, stopwatch
import time


class TestClass:
    @staticmethod
    @stopwatch()
    def decorated():
        time.sleep(0.25)


@stopwatch()
def decorated():
    time.sleep(0.50)


@stopwatch("auto_timer_custom_name")
def decorated_named():
    time.sleep(0.75)


def manual():
    Stopwatch.tick("manual_timer")
    time.sleep(1.0)
    Stopwatch.tock("manual_timer")


if __name__ == "__main__":
    TestClass.decorated()
    decorated()
    decorated_named()
    manual()

    Stopwatch.print_all()

    # Check for expected names and times in ms
    tolerance = 0.05  # 5%
    expected = {
        "TestClass.decorated": 250,
        "decorated": 500,
        "auto_timer_custom_name": 750,
        "manual_timer": 1000,
    }
    for k, v in Stopwatch.get_timings().items():
        if k not in expected:
            raise RuntimeError(f"{k} not in expected timings: {expected.keys()}")
        vTest = expected[k]
        if abs(v - vTest) > (tolerance * vTest):
            raise RuntimeError(f"Unexpected timing delta: {v} vs {vTest} with tolerence {tolerance*100:.1f} %")
