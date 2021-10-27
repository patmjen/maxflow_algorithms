import json
import os
import sys
from glob import glob
import argparse


def name_to_entry(f):
    return {
        'file_name': f,
        'file_type': f.split('.')[-1],
        'nbor_cap_type': 'int32',
        'term_cap_type': 'int32'
    }


def is_benchfile(f):
   return f.endswith('.bq') or f.endswith('.bbk')


def main(argv):
    parser = argparse.ArgumentParser(
        description="Build 'data_sets' entry for benchmark file"
    )
    parser.add_argument('file_glob', default='*', nargs='?',
                        help='pattern forwarded to glob (default: %(default)s)')
    parser.add_argument('--base_file', help='file with JSON data to insert into')
    args = parser.parse_args(argv)

    files = glob(args.file_glob)
    entries = [name_to_entry(os.path.abspath(f)) for f in files]

    if args.base_file is not None:
        with open(args.base_file) as in_file:
            bench_data = json.load(in_file)
        bench_data['data_sets'] = entries
        print(json.dumps(bench_data, indent=2))
    else:
        print(json.dumps(entries, indent=2))


if __name__ == '__main__':
    main(sys.argv[1:])

