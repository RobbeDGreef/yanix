import os, sys

def change(path, text, changed):
    defs = 0
    for dir, dirs, files in os.walk(path):
        for foundfile in files:
            content = ""
            try:
                with open(os.path.join(dir, foundfile), 'r') as f:
                    content = f.read()
                    #print(content)
                    content = content.replace(text, changed)
                    #print("changed: ------------------------------------------------")
                    #print(content)
                    #print("---------------------------------------------------------")
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
        path, text, changed = sys.argv[1:]
        print(path, text, changed)
        print("successfully changed text in:", change(path, text, changed), " files")
    except Exception as e:
        print('Expected format: [COMMAND] pathtoproject "tobechangedtext" "changedtext"')
        print("error: ", e)
        exit(1)

main()
