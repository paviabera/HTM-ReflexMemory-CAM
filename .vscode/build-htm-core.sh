rm -rfv htm.core/build
PIP_ROOT_USER_ACTION=ignore LD_LIBRARY_PATH=/usr/local/lib python3.11 -m ensurepip --upgrade \
  && PIP_ROOT_USER_ACTION=ignore LD_LIBRARY_PATH=/usr/local/lib python3.11 -m pip install setuptools packaging \
  && PIP_ROOT_USER_ACTION=ignore LD_LIBRARY_PATH=/usr/local/lib python3.11 -m pip install -r htm.core/requirements.txt \
  && cd htm.core \
  && PIP_ROOT_USER_ACTION=ignore LD_LIBRARY_PATH=/usr/local/lib python3.11 -m pip uninstall htm.core \
  && PIP_ROOT_USER_ACTION=ignore LD_LIBRARY_PATH=/usr/local/lib python3.11 setup.py install --force \
  && PIP_ROOT_USER_ACTION=ignore LD_LIBRARY_PATH=/usr/local/lib python3.11 -m pip install -r ../.devcontainer/requirements.txt







