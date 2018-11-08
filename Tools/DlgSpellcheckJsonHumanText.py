#!/usr/bin/env python3
# Copyright 2017-2018 Csaba Molnar, Daniel Butum

import os
import json
import sys
import argparse
import subprocess
import shlex
from pathlib import Path

# NOTE: This script is standalone does not include any libraries

class Colors:
    HEADER = '\033[95m'

    BLUE = '\033[0;36m'
    BLUE_LIGHT = '\033[1;36m'

    GREEN = '\033[0;32m'
    GREEN_LIGHT = '\033[1;32m'

    YELLOW = '\033[0;33m'
    YELLOW_LIGHT = '\033[1;33m'

    RED = '\033[0;31m'
    RED_LIGHT = '\033[1;31m'

    # No Color
    END = '\033[0m'


def print_newlines(nr = 1):
    if nr > 0:
        print('\n' * nr, end='')


def print_reset_color():
    if sys.stdout.isatty():
        print(Colors.END)


def _print_internal(color, string, **kwargs):
    if sys.stdout.isatty():
        # You're running in a real terminal
        prefix, suffix = color, Colors.END
    else:
        # You're being piped or redirected
        prefix, suffix = '', ''

    print(prefix + string + suffix, **kwargs)


def print_red(*args, **kwargs):
    _print_internal(Colors.RED, " ".join(map(str, args)), **kwargs)


def print_red_light(*args, **kwargs):
    _print_internal(Colors.RED_LIGHT, " ".join(map(str, args)), **kwargs)


def print_blue(*args, **kwargs):
    _print_internal(Colors.BLUE, " ".join(map(str, args)), **kwargs)


def print_blue_light(*args, **kwargs):
    _print_internal(Colors.BLUE_LIGHT, " ".join(map(str, args)), **kwargs)


def print_yellow(*args, **kwargs):
    _print_internal(Colors.YELLOW, " ".join(map(str, args)), **kwargs)


def print_yellow_light(*args, **kwargs):
    _print_internal(Colors.YELLOW_LIGHT, " ".join(map(str, args)), **kwargs)


def print_green(*args, **kwargs):
    _print_internal(Colors.GREEN, " ".join(map(str, args)), **kwargs)


def print_green_light(*args, **kwargs):
    _print_internal(Colors.GREEN_LIGHT, " ".join(map(str, args)), **kwargs)


def get_filename_from_path(path, include_extension=True):
    filename = os.path.basename(str(path))
    if include_extension:
        return filename

    file, extension = os.path.splitext(filename)
    return file


def exit_program(status):
    sys.exit(status)


def exit_program_error(message=None):
    if message is not None:
        print_red(message)
    exit_program(1)


def exit_program_success():
    exit_program(0)


def convert_path_to_absolute_if_not_already(path):
    if not os.path.isabs(path):
        return os.path.abspath(path)

    return path


def json_parse_file(path):
    """
    Returns a dictionary
    """
    try:
        with open(path) as fh:
            try:
                return json.load(fh)
            except ValueError as e:
                print_red("Can't parse file = `{}`. Error = `{}`".format(path, e))
                return None
    except IOError as e:
        print_red("Can't open file = `{}`. IOError = `{}`".format(path, e))
        return None


def json_save_dictionary(path, dictionary):
    try:
        with open(path, 'w') as fh:
            try:
                json.dump(dictionary, fh, indent=4)
            except ValueError as e:
                print_red("Can't save file = `{}`. Error = `{}`".format(path, e))
                return None
    except IOError as e:
        print_red("Can't open file = `{}`. IOError = `{}`".format(path, e))


class DlgEdge:
    def __init__(self, target_node_index, text):
        self.target_node_index = target_node_index
        self.text = text

    def __str__(self):
        return "DlgEdge(target_node_index = {}, text = {})".format(self.target_node_index, self.text)

    def __repr__(self):
        return str(self)


class DlgInnerEdge:
    def __init__(self, text, edge_text):
        self.text = text
        self.edge_text = edge_text

    def __str__(self):
        return "InnerEdge(text = {}, edge_text = {})".format(self.text, self.edge_text)

    def __repr__(self):
        return str(self)


class DlgNode:
    def __init__(self, node_index, text):
        self.node_index = node_index
        self.text = text
        self.edges = []

    def __str__(self):
        return "Node(node_index = {}, text = {}, edges =\n{})".format(self.node_index, self.text, "\n".join(str(n) for n in self.edges))

    def __repr__(self):
        return str(self)


class DlgSpeechSequenceNode:
    def __init__(self, node_index):
        self.node_index = node_index
        self.inner_edges = []
        self.edges = []


    def __str__(self):
        return "Node(node_index = {}, inner_edges = {}, edges =\n{})".format(self.node_index, "\n".join(str(n) for n in self.inner_edges), "\n".join(str(n) for n in self.edges))

    def __repr__(self):
        return str(self)


def is_path_json_human_text(file_path):
    if not os.path.isfile(file_path):
        return False

    path = Path(file_path)
    if "".join(path.suffixes) != ".dlg_human.json":
        return False

    # Collect all words in file
    all_words = set()


    return True


JSON_KEY_SPEECH_NODES = "SpeechNodes"
JSON_KEY_SPEECH_SEQUENCE_NODES = "SpeechSequenceNodes"
JSON_KEY_NODE_INDEX = "NodeIndex"
JSON_KEY_TEXT = "Text"
JSON_KEY_NODE_EDGES = "Edges"
JSON_KEY_NODE_SEQUENCE = "Sequence"
JSON_KEY_INNER_EDGE_TEXT = "EdgeText"
JSON_KEY_EDGE_TARGET_NODE_INDEX = "TargetNodeIndex"


def get_edges_from_node_json(node_json):
    edges = []
    if JSON_KEY_NODE_EDGES in node_json:
        for node_edge in node_json[JSON_KEY_NODE_EDGES]:
            if JSON_KEY_EDGE_TARGET_NODE_INDEX in node_edge and JSON_KEY_TEXT in node_edge:
                edge = DlgEdge(node_edge[JSON_KEY_EDGE_TARGET_NODE_INDEX], node_edge[JSON_KEY_TEXT])
                edges.append(edge)

    return edges

def get_inner_edges_from_node_json(node_json):
    inner_edges = []
    if JSON_KEY_NODE_SEQUENCE in node_json:
        for node_inner_edge in node_json[JSON_KEY_NODE_SEQUENCE]:
            if JSON_KEY_TEXT in node_inner_edge and JSON_KEY_INNER_EDGE_TEXT in node_inner_edge:
                inner_edge = DlgInnerEdge(node_inner_edge[JSON_KEY_TEXT], node_inner_edge[JSON_KEY_INNER_EDGE_TEXT])
                inner_edges.append(inner_edge)

    return inner_edges

def run_aspell_on_words(words_list_str):
    if not words_list_str:
        return ""

    try:
        # shlex.quote()
        # list -l en_us
        # pipe -l en_us --suggest --dont-byte-offsets --dont-guess
        process = subprocess.run("echo \"{}\" | aspell list -l en_us".format(words_list_str), check=True, cwd=None, universal_newlines=True, \
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    except ValueError as e:
        exit_program_error("ValueError = {}".format(e))
    except OSError as e:
        exit_program_error("OSError = {}".format(e))

    return process.stdout.strip()

def spellcheck_json_human_text(file_path):
    print_blue("Reading file = `{}`".format(file_path))

    file_json = json_parse_file(file_path)
    if not file_json:
        return

    # Speech nodes
    speech_nodes = []
    speech_sequence_node = []
    if JSON_KEY_SPEECH_NODES in file_json:
        for node_json in file_json[JSON_KEY_SPEECH_NODES]:

            # Read node
            if JSON_KEY_NODE_INDEX in node_json and JSON_KEY_TEXT in node_json:
                node = DlgNode(node_json[JSON_KEY_NODE_INDEX], node_json[JSON_KEY_TEXT])
                node.edges = get_edges_from_node_json(node_json)
                speech_nodes.append(node)


    # Speech sequence nodes
    if JSON_KEY_SPEECH_SEQUENCE_NODES in file_json:
        for node_json in file_json[JSON_KEY_SPEECH_SEQUENCE_NODES]:
            # Read node
            if JSON_KEY_NODE_INDEX in node_json and JSON_KEY_NODE_SEQUENCE in node_json:
                node = DlgSpeechSequenceNode(node_json[JSON_KEY_NODE_INDEX])
                node.edges = get_edges_from_node_json(node_json)
                node.inner_edges = get_inner_edges_from_node_json(node_json)
                speech_sequence_node.append(node)

    # Run spell checker on all words
    for node in speech_nodes:
        node_mistakes = run_aspell_on_words(node.text)
        if node_mistakes:
            print_yellow("Node Index = {}\n\tOriginal text = `{}`\n\tMistakes = `{}`\n".format(node.node_index, node.text, node_mistakes))

        warning_edges = []
        for edge in node.edges:
            edge_mistakes = run_aspell_on_words(edge.text)
            if edge_mistakes:
                warning_edges.append("TargetNodeIndex = {}\n\t\tOriginal text = `{}`\n\t\tMistakes = `{}`\n".format(edge.target_node_index, edge.text, edge_mistakes))

        if warning_edges and not node_mistakes:
            print_yellow("Node index = {}".format(node.node_index))

        for warning in warning_edges:
            print_yellow("\t" + warning)

    for node_sequence in speech_sequence_node:
        warnings_inner_edges = []
        for index, inner_edge in enumerate(node_sequence.inner_edges):
            warning = "InnerEdge Index = {}\n".format(index)
            found_mistake = False

            # Check text
            text_mistakes = run_aspell_on_words(inner_edge.text)
            if text_mistakes:
                found_mistake = True
                warning = "\tOriginal Text = `{}`\n\tMistakes = `{}`\n".format(inner_edge.text, text_mistakes)

            # Check EdgeText
            edge_mistakes = run_aspell_on_words(inner_edge.edge_text)
            if edge_mistakes:
                found_mistake = True
                warning = "\tOriginal EdgeText = `{}`\n\tMistakes = `{}`\n".format(inner_edge.edge_text, edge_mistakes)

            if found_mistake:
                warnings_inner_edges.append(warning)

        warning_edges = []
        for edge in node_sequence.edges:
            edge_mistakes = run_aspell_on_words(edge.text)
            if edge_mistakes:
                warning_edges.append("TargetNodeIndex = {}\n\t\tOriginal text = `{}`\n\t\tMistakes = `{}`\n".format(edge.target_node_index, edge.text, edge_mistakes))

        if warning_edges or warnings_inner_edges:
            print_yellow("Speech Sequence Node index = {}".format(node.node_index))

            for warning in warnings_inner_edges:
                print_yellow(warning)
            for warning in warning_edges:
                print_yellow("\t" + warning)


    print("\n" + "-" * 20)

def main(directory):
    print_blue("Finding json human text files inside directory = {}\n".format(directory))

    # Walk over all files in directory
    for path, subdirs, files in os.walk(directory):
        for name in files:
            full_filename = os.path.join(path, name)
            if is_path_json_human_text(full_filename):
                spellcheck_json_human_text(full_filename)
            else:
                print_yellow("Path = `{}` is not a file or not a valid json human text".format(full_filename))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', nargs="?", type=str, help='Directory containing all the json human text files', default="DialoguesJsonHumanText/")
    args = parser.parse_args()

    if not os.path.isdir(args.directory):
        exit_program_error("`{}` is not directory".format(args.directory))

    main(args.directory)
