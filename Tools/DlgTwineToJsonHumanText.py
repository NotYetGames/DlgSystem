#!/usr/bin/env python3
# Copyright 2017-2018 Csaba Molnar, Daniel Butum

import os
import sys
import argparse
import json
import uuid
import re
from pprint import pprint

from html.parser import HTMLParser
from html.entities import name2codepoint

# NOTE: This script is standalone does not include any libraries

DLG_JSON_HUMAN_EXTENSION = ".dlg_human.json"
ROOT_NODE_INDEX = -1


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
    print(Colors.END)


def print_red(*args, **kwargs):
    print(Colors.RED + " ".join(map(str, args)) + Colors.END, **kwargs)


def print_red_light(*args, **kwargs):
    print(Colors.RED_LIGHT + " ".join(map(str, args)) + Colors.END, **kwargs)


def print_blue(*args, **kwargs):
    print(Colors.BLUE + " ".join(map(str, args)) + Colors.END, **kwargs)


def print_blue_light(*args, **kwargs):
    print(Colors.BLUE_LIGHT + " ".join(map(str, args)) + Colors.END, **kwargs)


def print_yellow(*args, **kwargs):
    print(Colors.YELLOW + " ".join(map(str, args)) + Colors.END, **kwargs)


def print_yellow_light(*args, **kwargs):
    print(Colors.YELLOW_LIGHT + " ".join(map(str, args)) + Colors.END, **kwargs)


def print_green(*args, **kwargs):
    print(Colors.GREEN + " ".join(map(str, args)) + Colors.END, **kwargs)


def print_green_light(*args, **kwargs):
    print(Colors.GREEN_LIGHT + " ".join(map(str, args)) + Colors.END, **kwargs)


def string_to_int(string):
    try:
        return int(string)
    except ValueError:
        return None


class TwineNodeTag:
    NODE_START = "node-start"
    NODE_END = "node-end"
    NODE_VIRTUAL_PARENT = "node-virtual-parent"
    NODE_SPEECH = "node-speech"
    NODE_SPEECH_SEQUENCE = "node-speech-sequence"
    NODE_SELECTOR_FIRST = "node-selector-first"
    NODE_SELECTOR_RANDOM = "node-selector-random"

    @classmethod
    def get_all_tags(cls):
        return set([cls.NODE_START, cls.NODE_END, cls.NODE_VIRTUAL_PARENT, cls.NODE_SPEECH, cls.NODE_SPEECH_SEQUENCE, cls.NODE_SELECTOR_FIRST, cls.NODE_SELECTOR_RANDOM])

    @classmethod
    def is_valid_tag(cls, tag):
        return tag.lower() in cls.get_all_tags()

    @classmethod
    def has_valid_tags(cls, tags_list):
        tags_set = set([x.lower() for x in tags_list])
        common = cls.get_all_tags().intersection(tags_set)
        return bool(common)


class TwineEdgeData:
    EMPTY_TEXT_FLAG = "$$empty$$"

    def __init__(self):
        self.raw_target_node_index = None
        self.raw_text = None

        self.text = None
        self.target_node_index = None

    # The edge has empty text
    def is_empty_edge_text(self):
        return self.raw_text is None or self.EMPTY_TEXT_FLAG in self.raw_text.lower()

    def _parse_target_node_index(self):
        self.target_node_index = string_to_int(self.raw_target_node_index)

    def _parse_text(self):
        if self.is_empty_edge_text():
            self.text = ""
        else:
            self.text = self.raw_text

    def parse(self):
        self._parse_target_node_index()
        self._parse_text()

    def to_dict(self):
        if self.text is None or self.target_node_index is None or self.target_node_index < ROOT_NODE_INDEX:
            return {}

        return {
            "TargetNodeIndex": self.target_node_index,
            "Text": self.text
        }

    def __str__(self):
        return "TwineEdgeData(target_node_index = {}, text = `{}`)".format(self.target_node_index, self.text)

    def __repr__(self):
        return str(self)


class TwineNodeData:
    REGEX_EDGES = r"\[\[(.*)\]\]"

    def __init__(self):
        self.raw_name = None
        self.raw_data = None
        self.raw_tags = None

        # Computed from raw data
        self.node_index = None
        self.text = ""
        self.tags = []
        self.edges = []
        self.inner_edges = []

    def _parse_tags(self):
        self.tags = [x.lower() for x in  self.raw_tags.strip().split(" ")]

    def _parse_node_index(self):
        self.node_index = string_to_int(self.raw_name)

    def _parse_text(self):
        if not self.can_have_text():
            return

        index_edge_start = self.raw_data.find("[[")
        if index_edge_start == -1:
            # take whole string
            self.text = self.raw_data
        else:
            # Until the first
            self.text = self.raw_data[0:index_edge_start].strip()

    def _parse_edges(self):
        # Refuse to parse, because on some nodes we don't care about the edge text
        if not self.raw_data or not self.can_have_text_on_edges():
            return None

        matches = re.finditer(self.REGEX_EDGES, self.raw_data, re.MULTILINE | re.UNICODE)
        for index, match in enumerate(matches):
            group = match.group(1)
            # TODO make sure there are not multiple of these
            group_parts = group.split("|")
            if len(group_parts) != 2:
                print_yellow("Node Index = {} has an edge with len(group_parts) = {}. There must be exactly 2. Did you use `|` inside your edge?".format(self.node_index, len(group_parts)))
                continue

            edge = TwineEdgeData()
            edge.raw_text = group_parts[0]
            edge.raw_target_node_index = group_parts[1]
            edge.parse()
            self.edges.append(edge)

    # only for speech sequence nodese
    def _parse_inner_edges(self):
        if not self.is_node_speech_sequence() or not self.raw_data:
            return

        # TODO

    def parse(self):
        self._parse_tags()
        self._parse_node_index()
        self._parse_text()

        if not TwineNodeTag.has_valid_tags(self.tags):
            print_yellow("Node Index = {} does not have any valid tags = {}".format(self.node_index, self.tags))

        self._parse_edges()
        self._parse_inner_edges()

    def can_have_text(self):
        return self.is_node_speech() or self.is_node_virtual_parent()

    def can_have_text_on_edges(self):
        return self.is_node_start() or self.is_node_speech() or self.is_node_speech_sequence()

    def is_node_start(self):
        return TwineNodeTag.NODE_START in self.tags

    def is_node_end(self):
        return TwineNodeTag.NODE_END in self.tags

    def is_node_speech(self):
        return TwineNodeTag.NODE_SPEECH in self.tags

    def is_node_virtual_parent(self):
        return TwineNodeTag.NODE_VIRTUAL_PARENT in self.tags

    def is_node_speech_sequence(self):
        return TwineNodeTag.NODE_SPEECH_SEQUENCE in self.tags

    def is_node_selector(self):
        return self.is_node_selector_first() or self.is_node_selector_random()

    def is_node_selector_first(self):
        return TwineNodeTag.NODE_SELECTOR_FIRST in self.tags

    def is_node_selector_random(self):
        return TwineNodeTag.NODE_SELECTOR_RANDOM in self.tags

    def to_dict(self):
        if self.node_index is None or self.node_index < ROOT_NODE_INDEX:
            return {}

        if self.is_node_speech_sequence():
            # TODO
            return {}

        if self.can_have_text() or self.is_node_start():
            edges = []
            for edge in self.edges:
                edges.append(edge.to_dict())

            return {
                "NodeIndex": self.node_index,
                "Text": self.text,
                "Edges": edges
            }

        return {}

    def __str__(self):
        return "TwineNodeData(node_index = {}, tags = {}, text = `{}`, edges = {})".format(self.node_index, self.tags, self.text, self.edges)

    def __repr__(self):
        return str(self)


class TwineDocumentData:
    def __init__(self):
        self.raw_guid = None

        self.dialogue_name = None
        self.dialogue_guid = None
        self.nodes = []

    def _parse_dialogue_guid(self):
        # Convert to default Unreal uuid
        temp_uuid = uuid.UUID(self.raw_guid)
        self.dialogue_guid = temp_uuid.hex.upper()

    def parse(self):
        self._parse_dialogue_guid()

    def to_dict(self):
        if self.dialogue_name is None or self.dialogue_guid is None or not self.nodes:
            return {}

        speech_nodes = []
        speech_sequence_nodes = []
        for node in self.nodes:
            if node.is_node_speech_sequence():
                speech_sequence_nodes.append(node.to_dict())
            elif node.is_node_speech() or node.is_node_virtual_parent() or node.is_node_start():
                speech_nodes.append(node.to_dict())
            else:
                # Ignore
                pass

        return {
            "DialogueName": self.dialogue_name,
            "DialogueGuid": self.dialogue_guid,
            "SpeechNodes": speech_nodes,
            "SpeechSequenceNodes": speech_sequence_nodes
        }

    def __str__(self):
        return "TwineDocumentData(dialogue_name = {}, dialogue_guid = {}, nodes =\n{})".format(self.dialogue_name, self.dialogue_guid, "\n".join(str(n) for n in self.nodes))

    def __repr__(self):
        return str(self)


class TwineHtmlParser(HTMLParser):
    HTML_TAG_STORYDATA = "tw-storydata"
    HTML_TAG_PASSAGE_DATA = "tw-passagedata"

    HTML_ATTR_NAME = "name"
    HTML_ATTR_TAGS = "tags"
    HTML_ATTR_GUID = "ifid"

    def __init__(self):
        super().__init__()
        self.document = TwineDocumentData()
        self.current_tag = None
        self.current_node = None

    def handle_starttag(self, tag, attrs):
        # print("Start tag:", tag)
        self.current_tag = tag
        if tag == self.HTML_TAG_STORYDATA:
            # Data about dialogue
            for attr in attrs:
                attr_name, attr_value = attr
                if attr_name == self.HTML_ATTR_NAME:
                    self.document.dialogue_name = attr_value.strip()
                elif attr_name == self.HTML_ATTR_GUID:
                    self.document.raw_guid = attr_value.strip()

        elif tag == self.HTML_TAG_PASSAGE_DATA:
            # Data about each node
            self.current_node = TwineNodeData()
            self.document.nodes.append(self.current_node)

            for attr in attrs:
                attr_name, attr_value = attr
                if attr_name == self.HTML_ATTR_NAME:
                    self.current_node.raw_name = attr_value.strip()
                elif attr_name == self.HTML_ATTR_TAGS:
                    self.current_node.raw_tags = attr_value.strip()

    def handle_endtag(self, tag):
        if tag == self.HTML_TAG_STORYDATA:
            self.document.parse()
        elif tag == self.HTML_TAG_PASSAGE_DATA:
            self.current_node.parse()

        self.current_tag = None
        self.current_node = None
        # print("End tag  :", tag)

    def handle_data(self, data):
        if self.current_tag is None:
            return
        if self.current_node is None:
            return

        if self.current_tag == self.HTML_TAG_PASSAGE_DATA:
            self.current_node.raw_data = data.strip()

    def handle_comment(self, data):
        print("Comment  :", data)

    def handle_entityref(self, name):
        c = chr(name2codepoint[name])
        print("Named ent:", c)

    def handle_charref(self, name):
        if name.startswith('x'):
            c = chr(int(name[1:], 16))
        else:
            c = chr(int(name))
        print("Num ent  :", c)

    def handle_decl(self, data):
        print("Decl     :", data)


def exit_program(status):
    sys.exit(status)


def exit_program_error():
    exit_program(1)


def exit_program_success():
    exit_program(0)


def convert_path_to_absolute_if_not_already(path):
    if not os.path.isabs(path):
        return os.path.abspath(path)

    return path


def is_path_twine_file(path):
    filename = os.path.basename(str(path))
    file, extension = os.path.splitext(filename)

    if extension != ".html":
        return False

    # TODO Maybe parse the contents

    return True


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


def twine_parse_file(path):
    """
    Returns a dictionary
    """
    try:
        with open(path, 'r', encoding="utf8") as fh:
            parser = TwineHtmlParser()
            parser.feed(fh.read())
            return parser.document
    except IOError as e:
        print_red("Can't open file = `{}`. IOError = `{}`".format(path, e))
        return None


def export_twine_file_dlg_text_json(src_file_path, src_twine_dir_from, dst_json_dir):
    # Construct subdirectory we need to create our destination file
    src_dirname, src_filename = os.path.split(src_file_path)

    src_dirname_parts = src_dirname.split(os.sep)
    dst_dirname = None
    for index, part in enumerate(src_dirname_parts):
        if part == src_twine_dir_from:
            dst_dirname = os.sep.join(src_dirname_parts[index + 1:])
            break

    if dst_dirname is None:
        print_yellow("Can't find dst_dirname for src_file_path = `{}`".format(src_file_path))
        return

    # Ensure dirname exists in destination
    dst_dirname = os.path.join(dst_json_dir, dst_dirname)
    if not os.path.exists(dst_dirname):
        os.makedirs(dst_dirname, exist_ok=True)
        print_blue("Creating directory = `{}`".format(dst_dirname))
    if not os.path.isdir(dst_json_dir):
        print_yellow("Path = `{}` is not a directory. Ignoring".format(dst_dirname))
        return

    # Parse file
    print_blue("Parsing file = `{}`".format(src_file_path))
    twine_document =  twine_parse_file(src_file_path)
    if twine_document is None:
        print_yellow("Can't parse twine file = `{}`".format(src_file_path))
        return

    #print(twine_document)
    #print(twine_document.to_dict())

    json_human_content = twine_document.to_dict()
    if not json_human_content:
        print_yellow("Twine file = `{}` is corrupt or invalid. Can't parse any data".format(src_file_path))
        return

    # Write  file
    src_file, src_file_ext = os.path.splitext(src_filename)
    dst_file_path = os.path.join(dst_dirname, src_file) + DLG_JSON_HUMAN_EXTENSION
    print_blue("Writing file = `{}`".format(dst_file_path))
    json_save_dictionary(dst_file_path, json_human_content)
    print("")



def main(src_twine_dir, dst_json_dir):
    # Walk over all files in directory
    src_twine_dir = convert_path_to_absolute_if_not_already(src_twine_dir)
    dst_json_dir = convert_path_to_absolute_if_not_already(dst_json_dir)
    print_blue("Finding save files in src_twine_dir = {}\n".format(src_twine_dir))

    # Directory from where files
    src_twine_dir_from = os.path.basename(os.path.normpath(src_twine_dir))
    for path, subdirs, files in os.walk(src_twine_dir):
        for name in files:
            full_filename = os.path.join(path, name)
            if os.path.isfile(full_filename) and is_path_twine_file(full_filename):
                export_twine_file_dlg_text_json(full_filename, src_twine_dir_from, dst_json_dir)
            else:
                print_yellow("Path = `{}` is not a file or a twine file".format(full_filename))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('src_twine_dir', nargs='?', type=str, help='Source directory from where we get all the .html twine files', default="DialoguesTwine/")
    parser.add_argument('dst_json_dir', nargs='?', type=str, help='Destination directory where we store all the .dlg_human.json files',  default="DialoguesJsonHumanText/")

    args = parser.parse_args()
    src_twine_dir = args.src_twine_dir
    dst_json_dir = args.dst_json_dir
    if not os.path.exists(src_twine_dir):
        print_red("src_twine_dir = `{}` does not exist".format(src_twine_dir))
        exit_program_error()
    if not os.path.isdir(src_twine_dir):
        print_red("src_twine_dir = `{}` is not a directory".format(src_twine_dir))
        exit_program_error()

    if not os.path.exists(dst_json_dir):
        os.makedirs(dst_json_dir, exist_ok=True)
        print_blue("Creating dst_json_dir = `{}`".format(dst_json_dir))
    if not os.path.isdir(dst_json_dir):
        print_red("dst_json_dir = `{}` is not a directory".format(dst_json_dir))
        exit_program_error()

    main(src_twine_dir, dst_json_dir)

