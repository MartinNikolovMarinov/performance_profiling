#include <haversine.h>

f64 referenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 earthRadius) {
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;

    f64 dLat = core::degToRad(lat2 - lat1).value;
    f64 dLon = core::degToRad(lon2 - lon1).value;
    lat1 = core::degToRad(lat1).value;
    lat2 = core::degToRad(lat2).value;

    f64 a = square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));

    f64 Result = earthRadius * c;

    return Result;
}

void pairsToJSON(Pair* pairs, addr_size pCount, const char* filepath) {
    // printf("Serializing %zu pairs to %s\n", pCount, filepath);

    core::FileDesc fd =
        ValueOrDie(
            core::fileOpen(filepath,
                core::OpenMode::Read |
                core::OpenMode::Write |
                core::OpenMode::Create |
                core::OpenMode::Truncate),
            "Failed to open file for writing"
        );

    ValueOrDie(
        core::fileWrite(fd, "{\"pairs\":[\n", core::cptrLen("{\"pairs\":[\n"))
    );

    f64 hAverage = 0.0;

    constexpr addr_size FILE_BUFFER_SIZE = core::KILOBYTE * 4;
    char fileBuffer[FILE_BUFFER_SIZE];
    char* fileBufferPtr = fileBuffer;
    constexpr addr_size LINE_BUFFER_SIZE = 256;
    char lineBuffer[LINE_BUFFER_SIZE];

    for (addr_size i = 0; i < pCount; i++) {
        Pair& pair = pairs[i];

        lineBuffer[0] = 0;
        i32 n = sprintf(lineBuffer,
            "\t{\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, \"y1\":%.16f },\n",
            pair.x0, pair.y0, pair.x1, pair.y1);

        addr_size maxWrite = fileBuffer + FILE_BUFFER_SIZE - fileBufferPtr;

        if (maxWrite >= addr_size(n)) {
            fileBufferPtr = core::cptrCopy(fileBufferPtr, lineBuffer, n);
        }
        else {
            // The buffer is not large enough to hold the entire line

            // Fill the buffer to capacity:
            core::cptrCopy(fileBufferPtr, lineBuffer, maxWrite);

            // Flush the buffer to disk:
            ValueOrDie(core::fileWrite(fd, fileBuffer, FILE_BUFFER_SIZE));

            // Reset the buffer pointer to the beginning:
            fileBufferPtr = fileBuffer;

            // Copy the remaining characters:
            fileBufferPtr = core::cptrCopy(fileBufferPtr, lineBuffer + maxWrite, n - maxWrite);
        }

        // Null terminate the buffer:
        *fileBufferPtr = 0;

        // Calculate the reference distance
        hAverage += referenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1, EARTH_RADIUS);

        // Debug print:
        // printf("%s", lineBuffer);
    }

    if (fileBufferPtr != fileBuffer) {
        // Flush the buffer to disk:
        ValueOrDie(core::fileWrite(fd, fileBuffer, fileBufferPtr - fileBuffer));
    }

    hAverage = hAverage / pCount;
    printf("Average Haversine distance: %.16f\n", hAverage);

    ValueOrDie(
        core::fileWrite(fd, "]}\n", core::cptrLen("]}\n"))
    );
}

