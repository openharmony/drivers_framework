import os

os.system("npx webpack --mode=development")#development production

ss=open(r".\..\hcsVSCode\editor.html","r",encoding="utf8").read()
i1=ss.index("// update js code begin")+len("// update js code begin")+1
i2=ss.index("// update js code end")-1
destss=open(r".\dist\main.js","r",encoding="utf8").read()
# destss=destss.replace("\\n","\\\\n")
# destss=destss[19:-5]
ss=ss[:i1]+destss+ss[i2:]
open(r".\..\hcsVSCode\editor.html","w",encoding="utf8").write(ss)

print("replaced")