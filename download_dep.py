import urllib.request
import tarfile
import subprocess
import re

from pathlib import Path, PurePath
from enum import Enum

class Compression(Enum):
    GZ = 1
    XZ = 2


class Dependencies:

    def __init__(self, versions):
        self.path = Path('dependencies')
        self.path.mkdir(exist_ok=True)
        self.versions = versions
        self.glib_pattern = re.compile("(\d+\.\d+)\.\d+")

    def download(self, url, filename, comp):
        path_to_tar = self.path.as_posix() + '/' + filename
        urllib.request.urlretrieve(url, path_to_tar)
        method = {
                Compression.GZ: lambda p: tarfile.open(p, "r:gz"),
                Compression.XZ: lambda p: tarfile.open(p, "r:xz")
                }
        tar = method[comp](path_to_tar)
        tar.extractall(path=self.path.as_posix())
        tar.close()

    def build_jansson(self):
        version = self.versions['jansson']
        name = 'jansson-{}'.format(version)
        def b():
            tar_file = 'jansson-{}.tar.gz'.format(version)
            url = 'http://www.digip.org/jansson/releases/{}'.format(tar_file)
            self.download(url, tar_file, Compression.GZ)
            self.build(name)
        self.eventually_build(name, b)

    def build_glib(self):
        version = self.versions['glib']
        major = pattern = self.glib_pattern.search(version)
        name = 'glib-{}'.format(version)
        def b():
            if major is not None:
                tar_file = "glib-{}.tar.xz".format(version)
                url = 'https://ftp.gnome.org/pub/gnome/sources/glib/{}/{}'.format(major.group(1), tar_file)
                self.download(url, tar_file, Compression.XZ)
                self.build(name)
        self.eventually_build(name, b)

    def build_all(self):
        self.build_jansson()
        self.build_glib()

    def build(self, dep):
        p = PurePath("{}-build".format(dep))
        build_dir = self.path / p
        build_dir.mkdir(exist_ok=True)
        src_dir = self.path / (PurePath(dep))
        self.run_proc(["./configure", "--prefix={}".format(build_dir.absolute().as_posix()), "--enable-static"], src_dir)
        self.run_proc(["make"], src_dir)
        self.run_proc(["make", "install"], src_dir)

    def run_proc(self, args, cwd_dir):
        proc = subprocess.Popen(args, cwd=cwd_dir.as_posix())
        proc.wait()

    def is_already_downloaded(self, name):
        subdirs = [x for x in self.path.iterdir() if x.is_dir()]
        return name in map(lambda x: x.name, subdirs)

    def eventually_build(self, name, func):
        if self.is_already_downloaded(name):
            print(name + " already downloaded")
        else:
            func()


if __name__ == "__main__":
    versions = {
            'jansson': '2.11',
            'glib': '2.56.1'
            }
    dep = Dependencies(versions)
    dep.build_all()
