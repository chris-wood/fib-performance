import sys

def url_to_name(url):
    segments = []

    current = ""
    index = 0
    for c in url:
        if c == ".":
            segments.append(current)
            current = ""
        elif c == "/": 
            segments.append(current)
            current = ""
        elif c == "?":
            segments.append(current)
            segments.append(url[index + 1:])
            break
        else:
            current += c
        index += 1
    
    name = "ccnx:/" + "/".join(segments)
    while name.endswith("/"):
        name = name[:len(name) - 1]
    return name

def main(args):
    names = []
    for line in sys.stdin:
        if len(line) > 0:
            name = url_to_name(line)
            names.append(name)

    for n in names:
        print n

if __name__ == "__main__":
    main(sys.argv[1:])
