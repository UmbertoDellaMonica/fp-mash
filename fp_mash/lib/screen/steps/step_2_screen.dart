import 'package:flutter/material.dart';

class Step2Screen extends StatefulWidget {
  @override
  _Step2ScreenState createState() => _Step2ScreenState();
}

class _Step2ScreenState extends State<Step2Screen> {
  
  bool isStep2Completed = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Step 2'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              'Upload the files generated from Step 1 containing k-mers + frequency for genetic sequences 1 and 2.',
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
                      const Text('Genetic Sequence 1 k-mers', style: TextStyle(fontSize: 18.0)),
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
                      const Text('Genetic Sequence 2 k-mers', style: TextStyle(fontSize: 18.0)),
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
                // Implement common sequence finding logic here
                setState(() {
                  isStep2Completed = true;
                });
              },
              child: const Text('Find Common Sequences'),
            ),
            const SizedBox(height: 20),
            ElevatedButton(
              onPressed: isStep2Completed ? () {
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