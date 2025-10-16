#!/bin/python3

import os
import sys
import hashlib
import struct


def get_file_size_and_sha1(file_path):
    """
    计算文件的SHA1哈希值和文件大小。
    :param file_path: 文件的路径
    :return: 文件大小和SHA1哈希值的二进制摘要
    """
    file_size = os.path.getsize(file_path)

    sha1_hash = hashlib.sha1()
    with open(file_path, "rb") as f:
        # 逐块读取文件以处理大文件
        for byte_block in iter(lambda: f.read(4096), b""):
            sha1_hash.update(byte_block)

    sha1_digest = sha1_hash.digest()
    return file_size, sha1_digest


def create_xml_bin(xml_file):
    """
    将XML文件压缩成ZIP，然后将其与文件大小和SHA1哈希值打包成一个.bin文件。
    :param xml_file: 要处理的XML文件名
    """
    zip_file = "oah428_xml.zip"
    output_file = "XML.bin"

    # 步骤1: 检查并删除旧的ZIP压缩包
    if os.path.exists(zip_file):
        print(f"检测到旧的 '{zip_file}' 文件，正在删除...")
        os.remove(zip_file)
        print("删除成功。")

    # 步骤2: 将XML文件压缩成ZIP
    cmd = f"zip -9 -X {zip_file} {xml_file}"
    print(f"正在压缩文件，命令: {cmd}")
    os.system(cmd)

    # 步骤3: 获取ZIP文件的大小和SHA1哈希值
    file_size, sha1_digest = get_file_size_and_sha1(zip_file)
    print(f"ZIP文件大小: {file_size} 字节")
    print(f"SHA1 哈希值: {sha1_digest.hex()}")

    with open(zip_file, "rb") as f:
        zip_content = f.read()

    # 步骤4: 将所有内容写入.bin文件
    with open(output_file, "wb") as f:
        # 写入文件大小（4字节，小端字节序）
        f.write(struct.pack("<I", file_size))

        # 写入SHA1哈希值（20字节）
        f.write(sha1_digest)

        # 填充到128字节的头部
        padding_header = b'\x00' * (128 - 4 - len(sha1_digest))
        f.write(padding_header)

        # 写入ZIP文件的内容
        f.write(zip_content)

        # 计算总字节数，并检查是否能被4整除
        total_size = 128 + len(zip_content)
        remainder = total_size % 4

        # 如果不能被4整除，则添加填充字节
        if remainder != 0:
            padding_needed = 4 - remainder
            final_padding = b'\xFF' * padding_needed
            f.write(final_padding)
            print(f"文件大小 ({total_size} 字节) 不能被4整除。已添加 {padding_needed} 个 FF 字节进行填充。")

    print(f"\n成功生成 {output_file} 文件。")
    
    return output_file

def create_guf_file(xml_file):
    control_filename = "control.xml"
    package_filename = "package.zip"
    guf_filename = "XML.guf"


    cmd = f"zip -0 -X {package_filename} {control_filename} {xml_file}"
    print(f"generate package.zip: {cmd}")
    os.system(cmd)

    cmd = f"zip -0 -X {guf_filename} {control_filename} {package_filename}"
    print(f"generate XML.guf: {cmd}")
    os.system(cmd)


if __name__ == "__main__":
    # 在这里指定你的XML文件名
    # xml_filename = "reg_oah428_0903.xml"

    # 检查命令行参数
    if len(sys.argv) < 2:
        print("用法: python3 create_xml_bin.py <xml_filename>")
        sys.exit(1)

    xml_filename = sys.argv[1]
    print(xml_filename)

    # 检查文件是否存在
    if not os.path.exists(xml_filename):
        print(f"错误: 找不到文件 '{xml_filename}'。请确保文件与脚本位于同一目录下。")
        sys.exit(1)

    xml_file = create_xml_bin(xml_filename)

    # create guf
    create_guf_file(xml_file)
