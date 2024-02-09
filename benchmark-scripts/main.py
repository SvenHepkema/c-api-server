#!/usr/bin/python3

import datetime
import requests
import argparse
import logging

DEFAULT_PORT = 8080;
URL = f"http://localhost:{DEFAULT_PORT}/";

def empty_request(url: str, milliseconds: int) -> int:
    requests_made = 0
    start = datetime.datetime.now()

    microseconds = milliseconds * 1000 # convert milliseconds to microseconds

    while (datetime.datetime.now() - start).microseconds < microseconds:
        try:
            requests.get(url)
        except:
            logging.error(f"Sending an empty request to {url} failed.")
            break

        requests_made += 1

    logging.info(f"\t'{url}' within {milliseconds}ms there were \t{requests_made} requests")
    
    return requests_made


def main(args):
    logging.basicConfig(encoding='utf-8', level=logging.INFO)
    empty_request(URL + args.path, args.ms)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("path", help="path to fire requests at", type=str)
    parser.add_argument("--ms", help="duration in milliseconds", type=int, default=500)
    args = parser.parse_args()
    main(args)
