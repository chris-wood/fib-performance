import sys
import random
import string

def url_to_segments(url):
    segments = []

    if url.startswith("/"):
        url = url[1:]

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

    trimmed = [''.join(e for e in segment if e.isalnum()) for segment in segments]
    trimmed = filter(lambda s : len(s) > 0, trimmed)

    return trimmed

def segments_to_name(segments):
    name = "ccnx:/" + "/".join(segments)
    while name.endswith("/"):
        name = name[:len(name) - 1]
    return name

def url_to_name(url):
    segments = url_to_segments(url)
    if len(segments) > 0:
        return segments_to_name(segments)
    return None

def random_string(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))

def extend_name(segments, segment_sampler, segment_length_sampler):
    num_to_add = segment_sampler(len(segments))
    for i in range(num_to_add):
        segments.append(random_string(segment_length_sampler()))
    return segments_to_name(segments)

def generate_load(load):
    names = []
    count = 0
    for line in sys.stdin:
        if len(line) > 0:
            name = url_to_name(line)
            if name != None:
                names.append(name)
                count += 1
        if load != 0 and count > load:
            break
    for n in names:
        print n

# Sampled from the unibas data set
NUM_SEGMENTS_MEAN = 5.57
NUM_SEGMENTS_STDEV = 8.14
SEGMENT_LENGTH_MEAN = 10.39
SEGMENT_LENGTH_STDEV = 30.02

def segment_sampler(number, weight = 1.0):
    global NUM_SEGMENTS_MEAN
    global NUM_SEGMENTS_STDEV

    value = int(random.normalvariate(NUM_SEGMENTS_MEAN * weight, NUM_SEGMENTS_STDEV)) # values from Unibas data set -- fixed for now
    delta = value - number
    count = 0
    while delta < 0:
        count += 1
        value = int(random.normalvariate((NUM_SEGMENTS_MEAN * weight)+ count, NUM_SEGMENTS_STDEV)) # values from Unibas data set -- fixed for now
        delta = value - number
    return delta

def segment_length_sampler(weight = 1.0):
    global SEGMENT_LENGTH_MEAN
    global SEGMENT_LENGTH_STDEV

    value = int(random.normalvariate((SEGMENT_LENGTH_MEAN * 1.0), SEGMENT_LENGTH_STDEV)) # values from Unibas data set -- fixed for now
    count = 0
    while value < 0:
        count += 1
        value = int(random.normalvariate((SEGMENT_LENGTH_MEAN * 1.0) + count, SEGMENT_LENGTH_STDEV)) # values from Unibas data set -- fixed for now
    return value

def generate_test(load, increase_factor, component_weight = 1.0, number_weight = 1.0):
    base_names = []
    count = 0
    for line in sys.stdin:
        if len(line) > 0:
            segments = url_to_segments(line)
            base_names.append(segments)
            count += 1
            
        if load != 0 and count > load:
            break

    total = increase_factor * count

    for i in range(total):
        index = random.randint(0, count - 1)
        name = extend_name(base_names[index], lambda n : segment_sampler(n, number_weight), lambda : segment_length_sampler(component_weight))
        print name

def usage(args):
    print "usage: python %s [load | test] -- " % args[0]
    sys.exit(-1)

def main(args):
    if len(args) < 2:
        usage(args)

    load = int(args[1])
    cmd = args[2]

    if cmd == "load":
        generate_load(load)
    elif cmd == "test":
        factor = int(args[3])
        component_expansion = 1.0
        number_expansion = 1.0
        if len(args) >= 5:
            component_expansion = float(args[4])
        if len(args) >= 6:
            number_expansion = float(args[5])
        generate_test(load, factor, component_expansion, number_expansion)
    else:
        usage(args)

if __name__ == "__main__":
    main(sys.argv)
