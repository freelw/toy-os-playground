## 创建硬盘
```
qemu-img create -f raw ./disk10g.img 10G
```

```
( echo 'change vnc password' ; sleep 1 ; echo '0' ) | qemu-system-x86_64 -vnc :0,password=on -monitor stdio -drive file=./a.vfd,index=0,if=floppy,format=raw -boot a -drive file=./disk10g.img,index=1,if=virtio,format=raw -s -S
# -s：是 -gdb tcp::1234 的快捷方式，表示启动 GDB 远程调试服务器，监听 TCP 端口 1234
# -S：表示 Qemu 启动后立即停止 CPU 运行，等待 GDB 连接后才能继续执行。
```

```
gdb-multiarch
target remote:1234
b *0x7c00

#0x7ac是hd_info 可能会变
(gdb) x /16xb 0x7cac
0x7cac:	0x00	0x04	0xff	0xa0	0x3f	0xff	0xff	0x00
0x7cb4:	0xc8	0xff	0x3f	0x10	0xff	0x3f	0x3f	0x8d

```