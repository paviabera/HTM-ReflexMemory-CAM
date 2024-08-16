LD_LIBRARY_PATH=/usr/local/lib python3.11 -m ensurepip --upgrade \
  && LD_LIBRARY_PATH=/usr/local/lib python3.11 -m pip install setuptools packaging \
  && LD_LIBRARY_PATH=/usr/local/lib python3.11 -m pip install --no-cache-dir -r htm.core/requirements.txt \
  && cd htm.core \
  && LD_LIBRARY_PATH=/usr/local/lib python3.11 setup.py install \
  && LD_LIBRARY_PATH=/usr/local/lib python3.11 -m pip install --no-cache-dir -r ../.devcontainer/requirements.txt
