import os

def RecursiveInstall(env, target, dir):
    """
    This tool adds the builder:

        env.RecursiveInstall(target, path)

    This is useful for doing:

        k = env.RecursiveInstall(dir_target, dir_source)

    and if any thing in dir_source is updated the install is rerun

    It behaves similar to the env.Install builtin. However it expects
    two directories and correctly sets up the dependencies between each
    sub file instead of just between the two directories.

    Note in also traverses the in memory node tree for the source
    directory and can detect things that are not built yet. Internally
    we use the env.Glob function for this support.

    You can see the effect of this function by doing:

        scons --tree=all,prune

    and see the one to one correspondence between source and target
    files within each directory.
    """
    nodes = _recursive_install(env, dir)

    dir = env.Dir(dir).abspath
    target = env.Dir(target).abspath

    l = len(dir) + 1

    relnodes = [n.abspath[l:] for n in nodes]

    out = []
    for n in relnodes:
        t = os.path.join(target, n)
        s = os.path.join(dir, n)
        out.extend(env.InstallAs(env.File(t), env.File(s)))

    return out


def _recursive_install(env, path):
    """ Helper function for RecursiveInstall """
    nodes = env.Glob(os.path.join(path, '*'), strings=False)
    nodes.extend(env.Glob(os.path.join(path, '*.*'), strings=False))
    out = []
    for n in nodes:
        if n.isdir():
            out.extend(_recursive_install(env, n.abspath))
        else:
            out.append(n)

    return out


def generate(env, **kw):
    env.AddMethod(RecursiveInstall)


def exists(env):
    return True
