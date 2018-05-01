import urllib.request
import tarfile
import subprocess

from pathlib import Path, PurePath

class Dependencies:

    def __init__(self, versions):
        self.path = Path('dependencies')
        self.path.mkdir(exist_ok=True)
        self.versions = versions

    def download(self, url, filename):
        path_to_tar = self.path.as_posix() + '/' + filename
        urllib.request.urlretrieve(url, path_to_tar)
        tar = tarfile.open(path_to_tar, "r:gz")
        tar.extractall(path=self.path.as_posix())
        tar.close()

    def build_jansson(self):
        version = self.versions['jansson']
        name = 'jansson-{}'.format(version)
        if self.is_already_downloaded(name):
            print(name + " already downloaded")
        else:
            tar_file = 'jansson-{}.tar.gz'.format(version)
            url = 'http://www.digip.org/jansson/releases/{}'.format(tar_file)
            self.download(url, tar_file)
            self.build(name)

    def build_all(self):
        self.build_jansson()

    def build(self, dep):
        p = PurePath("{}-build".format(dep))
        build_dir = self.path / p
        build_dir.mkdir(exist_ok=True)
        src_dir = self.path / (PurePath(dep))
        self.run_proc(["./configure", "--prefix={}".format(build_dir.absolute().as_posix())], src_dir)
        self.run_proc(["make"], src_dir)
        self.run_proc(["make", "install"], src_dir)

    def run_proc(self, args, cwd_dir):
        proc = subprocess.Popen(args, cwd=cwd_dir.as_posix())
        proc.wait()

    def is_already_downloaded(self, name):
        subdirs = [x for x in self.path.iterdir() if x.is_dir()]
        return name in map(lambda x: x.name, subdirs)

if __name__ == "__main__":
    versions = {
            'jansson': '2.11'
            }
    dep = Dependencies(versions)
    dep.build_all()
