#！/bin/bash
current_dir=$(cd $(dirname $0); pwd)
python3 ${current_dir}/beast_web_server.py
