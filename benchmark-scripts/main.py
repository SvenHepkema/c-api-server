#!/usr/bin/python3

import datetime
import requests
import argparse
import logging
import threading

DEFAULT_WAIT_MS = 200
DEFAULT_PORT = 8080;
URL = f"http://localhost:{DEFAULT_PORT}/";



def empty_request_executor(index: int, start: datetime.datetime, duration_ms: int, url: str, results: list) -> None:
    requests_made = 0

    while datetime.datetime.now() < start:
        pass

    while (datetime.datetime.now() - start).microseconds < duration_ms:
        try:
            requests.get(url)
        except:
            logging.error(f"Sending an empty request to {url} failed.")
            break
        requests_made += 1

    results[index] = requests_made
    


def empty_request(url: str, milliseconds: int, n_threads: int) -> int:
    microseconds = milliseconds * 1000 # convert milliseconds to microseconds

    start = datetime.datetime.now() + datetime.timedelta(milliseconds=(DEFAULT_WAIT_MS))
    requests_made = 0
    threads = []
    results = [0] * n_threads

    for i in range(n_threads):
        threads.append(threading.Thread(target=empty_request_executor, args=(i, start, microseconds, url, results)))

    for thread in threads:
        thread.start()

    for thread in threads:
        thread.join()
    
    requests_made = sum(results)

    logging.info(f"\t'{url}' within {milliseconds}ms {n_threads} threads made \t{requests_made} requests")
    
    return requests_made


def main(args):
    logging.basicConfig(encoding='utf-8', level=logging.INFO)
    empty_request(URL + args.path, args.ms, args.n_threads)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("path", help="path to fire requests at", type=str)
    parser.add_argument("--ms", help="duration in milliseconds", type=int, default=500)
    parser.add_argument("--n_threads", help="number of threads", type=int, default=10)
    args = parser.parse_args()
    main(args)
