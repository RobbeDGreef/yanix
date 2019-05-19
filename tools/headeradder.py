import os, sys

def change(path, findtext, header):
    defs = 0
    for dir, dirs, files in os.walk(path):
        for foundfile in files:
            content = ""
            try:
                with open(os.path.join(dir, foundfile), 'r') as f:
                    content = f.read()
                    if findtext in content:
                        with open(os.path.join(dir, foundfile), 'w') as wf:
                            wf.write(content)
                            wf.close()
                    f.close()
                    defs += 1
            except Exception as e:
                print("warning: ", e)
                pass
    return defs

def main():
    try:
        path, findtext, header = sys.argv[1:]
        print(path, findtext, header)
        print("successfully added headers in:", change(path, text, changed), " files")
    except Exception as e:
        print('Expected format: [COMMAND] pathtoproject "findtext" "header"')
        print("error: ", e)
        exit(1)

main()
