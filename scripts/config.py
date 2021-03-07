#!/usr/bin/python3
#
# Read a config file and default config settings and emit a .config.h and .config.mk
# file.
#
# This aims to function a bit like Kconfig in the Linux kernel, but it's far less
# powerful or good. However, there's a _lot_ less stuff for us to worry about
# configing.
#

import os
import re
import sys

# Global configs map; the parsing process fills this list up. We'll subsequently
# iterate over this and resolve dependencies.
#
# The map is name -> Config(name, ...)
configs = { }

# A map of default overrides; the map is name -> override
default_overrides = { }

class DepException(Exception):
    """
    Raised if a circular dependency is detected.
    """
    pass

class Config(object):
    """
    A config, along with a list of dependent configs. There's really two lists:
    the unresolved, textual configs, and the resolved Config list.
    """

    def __init__(self, name, default, deps=[]):
        self.name        = name
        self.default     = default.lower()
        self.__text_deps = deps

    def __str__(self):

        dep_str = ''
        deps = ' '.join(self.deps)
        not_deps = ' '.join(['!' + x for x in self.not_deps])

        if deps or not_deps:
            dep_str = '| ' + deps + not_deps

        conf = 'CONFIG_%s=%s' % (self.name, self.default)

        return '%-30s %s' % (conf, dep_str)

    def __lt__(self, target):
        return self.name < target.name

    def to_mk(self):
        return 'CONFIG_%-25s = %s' % (self.name, self.enabled())

    def to_h(self):
        return '#define CONFIG_%-25s %s' % (self.name, self.enabled())

    def resolve(self):
        """
        Go over our list of text deps and convert them into links to actual
        Configs. We have two lists, deps and not_deps, depending on an
        inversion - e.g depending on something _not_ set.
        """

        global configs

        self.deps = { }
        self.not_deps = { }

        dep_list = self.deps

        for d in self.__text_deps:
            if d[0] == '!':
                dep_list = self.not_deps
                d = d[1:]

            conf = configs.get(d, None)

            if not conf:
                print('%s has unknown dependency: %s' % (self.name, d))
                return False

            dep_list[conf.name] = conf

        return True

    def enabled(self, seen_deps=None):
        """
        Check if this config enabled; the rsult of this is:

            self.default && self.deps[0] && self.deps[1] && ...

        For all deps. deps can be empty, of course. The only concern here
        is making sure to detect circular dependencies. seen_deps keeps a
        list of all dependencies seen in the chain. As we call this function
        recursively we can pass in the accumulated depenency list.
        """

        if not seen_deps:
            seen_deps = { }

        # Make sure we are not in the seen deps list; this would imply a
        # circular dependency.
        if self.name in seen_deps:
            print('%s: circular dependency detected.' % self.name)
            print(seen_deps)
            raise DepException()

        seen_deps[self.name] = self

        # Easy: if the default is no, we are done.
        if self.default != 'yes':
            return 'n'

        # Now start checking the dep lists. They must all be satisfied for
        # this to be enabled.
        for d in self.deps:
            if self.deps[d].enabled(seen_deps) == 'n':
                return 'n'

        for d in self.not_deps:
            if self.not_deps[d].enabled(seen_deps) == 'y':
                return 'n'

        return 'y'

def verif(name, default):
    """
    Verify that a name/default/deps list is valid.

      name    - Must be alphanumeric. E.g valid C syntax.
      default - a 'yes' or 'no'.
      deps    - A list of alphanumerics, with a starting '!' as an exception.
                This gets caught during resolve, though.

    Returns False if an error is encountered.
    """

    if not re.match(r'\w+', name):
        print('Parse error!')
        print('  Invalid name: %s\n' % name)
        return False

    if default != 'yes' and default != 'no':
        print('Parse error!')
        print('  Default is invalid: %s' % default)
        print('  Default must be \'yes\' or \'no\'\n')
        return False

    return True

def clean_line(line):
    # Get rid of comments; everything after the first '#' is ignored.
    line_data = line.split('#', 1)

    # Now remove extra whitespace if present.
    return line_data[0].strip()

def parse_overrides(override_file):
    """
    Given a file of:

       # Config     Override value
       SOME_THING   yes

    Build a map of these. We'll then update the defaults before creating the
    config files.
    """

    global default_overrides

    filp = open(override_file, "r")

    for line in filp:
        line = clean_line(line)

        # Empty line, on to the next.
        if not line:
            continue

        override_data = line.split()
        if len(override_data) != 2:
            print('Invalid config line: \'%s\'' % line)
            return False

        name, override = override_data

        if not verif(name, override):
            print('Invalid config line: \'%s\'' % line)
            return False

        default_overrides[name] = override

    filp.close()

    return True

def set_overrides():
    """
    From the set of overrides we loaded, update the list of configs. This
    only changes their default; it won't necessarily impact their actual
    output.
    """

    global default_overrides
    global configs

    for o in default_overrides:
        if o not in configs:
            continue

        conf = configs[o]
        conf.default = default_overrides[o]

def print_config_mk():
    """
    From the configs, print out a makefile config file that can be included
    by makefiles.
    """

    configmk = open('.config.mk', 'w')

    configmk.write('# AUTOGENERATED from config.py. DO NOT EDIT!\n\n')

    for c in sorted(configs.values()):
        if c.enabled() == 'y':
            configmk.write(c.to_mk() + '\n')

    configmk.close()

def print_config_h():
    """
    Like the mk version, just in CPP syntax.
    """

    configh = open('.config.h', 'w')

    configh.write('/* AUTOGENERATED from config.py. DO NOT EDIT! */\n\n')
    configh.write('#ifndef __CONFIG_H__\n')
    configh.write('#define __CONFIG_H__\n\n')

    for c in sorted(configs.values()):
        if c.enabled() == 'y':
            configh.write(c.to_h() + '\n')

    configh.write('\n#endif\n')
    configh.close()

def parse_line(line):
    """
    Parse a line of a single config file. Config file lines are generally a single
    configuration name and a default value, e.g:

        # Config         Default
        # ------         -------
        MY_CONFIG        yes
        ANOTHER_CONFIG   no

    You may also specify dependencies as a white space separated list; this will only
    enable the config if the default is yes and the dependency is also set. You can
    invert a dependency with a ! but that's it for now; there's an implicit AND between
    them all. Example:

        MY_CONFIG        yes       DEP_CONFIG !INVERTED_DEP
    """

    global configs

    data = clean_line(line)

    # Empty line, we are done.
    if not data:
        return True

    # Now we have, theoretically, a config line. We'll split by whitespace
    # and consider the first two elements as name/default and subsequent
    # elements as dependencies, if any.
    data_array = data.split()

    name    = data_array.pop(0)
    default = data_array.pop(0)
    deps    = data_array

    # Special case: handle an include directive; it's recursive. For an
    # include directive 'default' holds the target file to include.
    if name == 'include':
        return parse_config_file(default)

    if not verif(name, default):
        return False

    configs[name] = Config(name, default, deps)

    return True

def parse_config_file(rel_file):
    """
    Parse a single configuration file of name rel_file. Can be called
    recursively.

    Returns True on success, False otherwise.
    """

    line_nr = 1
    cwd = os.getcwd()

    # Move to the directory in which file_name resides. This way include
    # directives are always relative to the file doing the include.
    dir_name, file_name = os.path.split(rel_file)

    if dir_name:
        os.chdir(dir_name)

    filp = open(file_name, "r")

    for line in filp:
        if not parse_line(line):
            print('Failed to parse %s:%d' % (rel_file, line_nr))
            return False

        line_nr += 1

    filp.close()

    os.chdir(cwd)

    return True

if len(sys.argv) > 3:
    print('Missing config file!')
    exit(1)

config_spec = sys.argv[1]

if len(sys.argv) == 3:
    if not parse_overrides(sys.argv[2]):
        exit(1)

if not parse_config_file(config_spec):
    exit(1)

# Ignore any errors here.
set_overrides()

for c in configs.values():
    if not c.resolve():
        exit(1)

# Now that we've resolved our dependencies, print out the necessary files.
print_config_mk()
print_config_h()
