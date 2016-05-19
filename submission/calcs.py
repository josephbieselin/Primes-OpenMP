



def process(f):
    fh = open(f, 'r')
    fp = open("timing.txt", "a")
    data = []
    line = ""
    for i in range(7):
        for j in range (6):
            fh.readline()
            line = fh.readline()
            line = line.strip()
            line_split = line.split()
            time = line_split[len(line_split) - 1]
            fp.write(time)
            fp.write("\n")
        fp.write("\n\n")

    fp.write("\n\n\n")

    fp.close()
    fh.close()


def main():
    f1 = "timing_1000.txt"
    f2 = "timing_10000.txt"

    process(f1)
    process(f2)

if __name__ == '__main__':
    main()
