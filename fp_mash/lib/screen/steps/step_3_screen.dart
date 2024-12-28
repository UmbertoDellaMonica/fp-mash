import 'package:flutter/material.dart';

class Step3Screen extends StatefulWidget {
  @override
  _Step3ScreenState createState() => _Step3ScreenState();
}

class _Step3ScreenState extends State<Step3Screen> {
  bool isStep3Completed = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Step 3 - Lyn2vec Fingerprints '),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              'Upload the files splitted from Step 2. For each file generate a finger print file',
              textAlign: TextAlign.center,
              style: TextStyle(fontSize: 18.0),
            ),
            const SizedBox(height: 20),
            Row(
              children: <Widget>[
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.center,
                    children: <Widget>[
                      const Text('Genetic Sequence 1 Splitted by k-mers', style: TextStyle(fontSize: 18.0)),
                      ElevatedButton(
                        onPressed: () {
                          // Implement file picker here
                        },
                        child: const Text('Upload File'),
                      ),
                    ],
                  ),
                ),
                const SizedBox(width: 20),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.center,
                    children: <Widget>[
                      const Text('Genetic Sequence 2 Splitted by k-mers', style: TextStyle(fontSize: 18.0)),
                      ElevatedButton(
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
            ElevatedButton(
              onPressed: () {
                // TODO: Implement the following actions:
                // - Retrieve the file splitted_1.txt for genetic sequence 1
                // - Retrieve the file splitted_2.txt for genetic sequence 2
                // - Call lyn2vec to generate the fingerprints using Python code
                // - If everything is successful, display a success toast

                setState(() {
                  isStep3Completed = true;
                });
              },
              child: const Text('Find Common Sequences'),
            ),
            const SizedBox(height: 20),
            ElevatedButton(
              onPressed: isStep3Completed ? () {
                // Navigate to the next step
              } : null,
              child: const Text('Next >'),
            ),
          ],
        ),
      ),
    );
  }
}
