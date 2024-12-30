import 'package:flutter/material.dart';

class Step1Screen extends StatefulWidget {
  @override
  _Step1ScreenState createState() => _Step1ScreenState();
}

class _Step1ScreenState extends State<Step1Screen> {
  
  bool isTextInput1 = true;
  bool isTextInput2 = true;
  bool isStep1Completed = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Step 1'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              'Insert the FASTA genetic sequence in your preferred mode to extract k-mers + frequency using DSK.',
              textAlign: TextAlign.center,
              style: TextStyle(fontSize: 18.0),
            ),
            const SizedBox(height: 20),
            Row(
              children: <Widget>[

                /// Sequence Genetic 1 - Insert Input 
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.center,
                    children: <Widget>[
                      const Text('Genetic Sequence 1', style: TextStyle(fontSize: 18.0)),
                      Row(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: <Widget>[
                          Checkbox(
                            value: isTextInput1,
                            onChanged: (bool? value) {
                              setState(() {
                                isTextInput1 = value!;
                              });
                            },
                          ),
                          const Text('Text Input'),
                          const SizedBox(width: 10),
                          Checkbox(
                            value: !isTextInput1,
                            onChanged: (bool? value) {
                              setState(() {
                                isTextInput1 = !value!;
                              });
                            },
                          ),
                          const Text('File Input'),
                        ],
                      ),
                      isTextInput1
                          ? const TextField(
                              maxLines: 5,
                              decoration: InputDecoration(
                                border: OutlineInputBorder(),
                                hintText: 'Enter genetic sequence',
                              ),
                            )
                          : ElevatedButton(
                              onPressed: () {
                                // Implement file picker here
                              },
                              child: const Text('Upload File'),
                            ),
                    ],
                  ),
                ),
                const SizedBox(width: 20),
                /// Sequence Genetic 2 - Insert Input 
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.center,
                    children: <Widget>[
                      const Text('Genetic Sequence 2', style: TextStyle(fontSize: 18.0)),
                      Row(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: <Widget>[
                          Checkbox(
                            value: isTextInput2,
                            onChanged: (bool? value) {
                              setState(() {
                                isTextInput2 = value!;
                              });
                            },
                          ),
                          const Text('Text Input'),
                          const SizedBox(width: 10),
                          Checkbox(
                            value: !isTextInput2,
                            onChanged: (bool? value) {
                              setState(() {
                                isTextInput2 = !value!;
                              });
                            },
                          ),
                          const Text('File Input'),
                        ],
                      ),
                      isTextInput2
                          ? const TextField(
                              maxLines: 5,
                              decoration: InputDecoration(
                                border: OutlineInputBorder(),
                                hintText: 'Enter genetic sequence',
                              ),
                            )
                          : ElevatedButton(
                              onPressed: () {
                                // Implement file picker here
                              },
                              child: const Text('Upload File'),
                            ),
                    ],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 20),

            /// Execute Action 
            ElevatedButton(
              onPressed: () {

                // PRE-CONDITION:
                // - If the user enters the genetic sequence in text format instead of via file, then
                //   create the respective files seq-genetic1.fasta and seq-genetic2.fasta and then
                //   pass them to the DSK program.
                //
                // - Otherwise, directly pass the specific files that were uploaded.

                // TODO: Implement k-mer identification logic here using the following steps:
                // 1. Execute DSK to obtain k-mers in HDF5 format:
                //    ./dsk -file <file.fasta> -kmer-size <k>
                //    Example: ./dsk -file A1.fa,A2.fa,A3.fa -kmer-size 31
                //    (The output is a .h5 file)
                //
                // 2. Convert the .h5 file to .txt format:
                //    ./dsk2ascii -file output.h5 -out output.txt
                //    (This needs to be done for both genetic sequences 1 and 2)
                //
                // 3. The resulting files will be used in Step 2 of the application.

                setState(() {
                  isStep1Completed = true;
                });
              },
              child: const Text('Execute k-mer + frequency identification'),
            ),
            const SizedBox(height: 20),

            /// Next - Step -> Going to Step 2 
            ElevatedButton(
              onPressed: isStep1Completed ? () {
                // Navigate to the next step in the pipeline
                Navigator.pushNamed(context, '/step2');
              } : null,
              child: const Text('Next >'),
            ),
          ],
        ),
      ),
    );
  }
}
